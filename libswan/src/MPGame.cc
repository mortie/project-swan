#include "MPGame.h"
#include "Entity.h"
#include "capnp/message.h"
#include "common.h"
#include "traits/BodyTrait.h"
#include "traits/PlayerControllerTrait.h"
#include "EntityCollectionImpl.h" // IWYU pragma: keep

#include <imgui/imgui.h>
#include <memory>
#include <kj/io.h>
#include <capnp/serialize-packed.h>
#include <swan/constants.h>
#include <swan/log.h>

namespace Swan {

static constexpr float TICK_DELTA = 1.0 / TICK_RATE;

MPGame::MPGame(std::function<bool()> recompileMods, HashMap<ModInfo> mods):
	recompileMods_(recompileMods),
	mods_(std::move(mods))
{}

void MPGame::onMouseMove(float x, float y)
{
	Vec2 pixPos{x, y};
	mousePos_ = (pixPos / cam_.size.as<float>()) * renderer_.winScale();
	pixPos -= uiCam_.size / 2;
	mouseUIPos_ = (pixPos / uiCam_.size / uiCam_.zoom * 2) * renderer_.winScale();
	gui_.onMouseMove(mouseUIPos_);
	hasMouseMoved_ = true;
}

void MPGame::onScrollWheel(float dy)
{
	cam_.zoom += dy * 0.05f * cam_.zoom;

	float zoomLim = debug_.godMode ? 0.002 : 0.0175;
	if (cam_.zoom > 1) {
		cam_.zoom = 1;
	}
	else if (cam_.zoom < zoomLim) {
		cam_.zoom = zoomLim;
	}
}

void MPGame::onViewportSize(int w, int h)
{
	cam_.size = {w, h};
	uiCam_.size = {w, h};
}

Vec2 MPGame::getMousePos()
{
	return (getMouseScreenPos() * 2 - renderer_.winScale()) / cam_.zoom + cam_.pos;
}

TilePos MPGame::getMouseTile()
{
	auto pos = (getMouseScreenPos() * 2 - renderer_.winScale()) / cam_.zoom + cam_.pos;
	return TilePos{(int)floor(pos.x), (int)floor(pos.y)};
}


void MPGame::update(float dt)
{
	inputHandler_.beginFrame();
	inputHandler_.endFrame();

	tickTimer_ += dt;
	if (tickTimer_ >= TICK_DELTA) {
		tickTimer_ -= TICK_DELTA;
		tick(TICK_DELTA);
	}

	renderer_.update(dt);

	auto *controller = player_.as<PlayerControllerTrait>();
	if (controller) {
		auto override = plane_->entities().overrideCurrentEntity(player_);
		controller->controlPlayer(plane_->getContext(), dt);

		// Make camera follow player
		auto body = player_.trait<BodyTrait>();
		auto camTarget = body->pos + body->size / 2;
		auto camSqDist = (cam_.pos - camTarget).squareLength();
		if (camSqDist > 20 * 20) {
			cam_.pos = camTarget;
		} else {
			constexpr float HALF_LIFE = 0.05;
			cam_.pos = {
				lerpSmooth(cam_.pos.x, camTarget.x, HALF_LIFE, dt),
				lerpSmooth(cam_.pos.y, camTarget.y, HALF_LIFE, dt),
			};
		}
	}
}

void MPGame::draw()
{
	ImGui::Begin(
		"Connection Status", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoResize);
	ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);

	const char *stateStr = "<unknown>";
	auto state = client_.state();
	switch (state.tag) {
	case MPClient::CONNECTING:
		stateStr = "Connecting...";
		break;
	case MPClient::HANDSHAKING:
		stateStr = "Handshaking...";
		break;
	case MPClient::CONNECTED:
		stateStr = "Connected.";
		break;
	case MPClient::CLOSED:
		stateStr = "Closed.";
		break;
	case MPClient::KICKED:
		stateStr = "Kicked.";
		break;
	case MPClient::SHUTDOWN:
		stateStr = "Server shut down.";
		break;
	case MPClient::ERROR:
		stateStr = "Error.";
		break;
	}

	ImGui::Text("Connection state: %s", stateStr);
	if (!state.reason.empty()) {
		ImGui::Text("Reason: %s", state.reason.c_str());
	}

	ImGui::End();

	renderer_.setCull({
		.pos = cam_.pos,
		.size = {1 / cam_.zoom, 1 / cam_.zoom},
	});

	renderer_.clear();

	if (plane_) {
		renderer_.setBackgroundColor(plane_->worldGen_->backgroundColor(cam_.pos));
		plane_->keepChunksActiveAround(cam_.pos);
		plane_->draw(renderer_, cam_.pos);
	}

	auto *controller = player_.as<PlayerControllerTrait>();
	if (controller) {
		controller->drawUI(plane_->getContext(), renderer_);
	}
}

void MPGame::render()
{
	renderer_.render(cam_);
	renderer_.renderUI(uiCam_);
}

void MPGame::onQuit()
{
	info << "Sending quit message";
	client_.end();
}

void MPGame::connect(MPClient::Options opts)
{
	opts.requestWorld = true;
	client_.connect(std::move(opts));
}

void MPGame::tick(float dt)
{
	client_.tick(dt);

	mp_proto::ServerToClient::Reader r;
	while (client_.receive(r)) {
		onMessageFromServer(r);
	}
}

void MPGame::onMessageFromServer(mp_proto::ServerToClient::Reader &r)
{
	if (r.isWorldSync()) {
		auto sync = r.getWorldSync();

		// Resolve the paths of all the mods
		std::vector<std::string> modPaths;
		modPaths.reserve(sync.getModIDs().size());
		bool hasAllMods = true;
		for (auto modID: sync.getModIDs()) {
			auto mod = mods_.find(modID.cStr());
			if (mod == mods_.end()) {
				panic << "Missing mod: " << modID.cStr();
				hasAllMods = false;
			}

			modPaths.push_back(mod->second.path);
			info << "Server requested mod: " << modID.cStr();
		}

		if (!hasAllMods) {
			client_.end();
			return;
		}

		std::vector<std::string> namesByID;
		namesByID.reserve(sync.getNamesByID().size());
		for (auto name: sync.getNamesByID()) {
			namesByID.push_back(name);
		}

		data_ = std::make_unique<WorldData>();
		data_->loadMods(modPaths);
		data_->buildResources(renderer_, namesByID);

		// Initiate input handler
		initInputHandler();

		// Init mods
		for (auto &mod: data_->mods_) {
			mod.mod_->start(*data_, *this);
		}

		// Make world generator
		auto worldGenName = sync.getCurrentPlane().getWorldGen().cStr();
		auto worldGenIt = data_->worldGenFactories_.find(worldGenName);
		if (worldGenIt == data_->worldGenFactories_.end()) {
			panic << "Missing world generator: " << worldGenName;
			client_.end();
			return;
		}
		auto worldGen = worldGenIt->second.create(*data_, sync.getWorldSeed());

		// Make entity collections
		std::vector<std::unique_ptr<EntityCollection>> colls;
		colls.reserve(data_->entCollFactories_.size());
		for (auto &fact: data_->entCollFactories_) {
			colls.emplace_back(fact.second.create(fact.second.name));
		}

		plane_ = std::make_unique<WorldPlane>(
			sync.getCurrentPlaneIndex(), data_.get(), this,
			std::move(worldGen), std::move(colls));
		plane_->deserialize(sync.getCurrentPlane());
		player_.deserialize(plane_->getContext(), sync.getPlayerRef());

		info << "Successfully performed initial world sync.";
	} else if (r.isTick()) {
		auto tick = r.getTick();

		Ctx ctx = plane_->getContext();
		for (auto update: tick.getUpdatedEntityCollections()) {
			auto colls = plane_->entities().collections();
			size_t index = update.getIndex();
			if (index >= colls.size()) {
				warn << "Got update for out-of-range collection: " << index;
				continue;
			}

			plane_->deserializeCollectionUpdates(*colls[index], update);
		}

		{ // Deserialize world gen data
			auto data = tick.getWorldGenData();
			kj::ArrayInputStream stream(data);
			capnp::PackedMessageReader reader(stream);
			plane_->worldGen_->deserialize(ctx, reader);
		}

		for (auto change: tick.getTileChanges()) {
			auto pos = TilePos{change.getPos().getX(), change.getPos().getY()};
			plane_->tiles().forceSetID(pos, change.getNewTile());
		}

		for (auto change: tick.getBackgroundChanges()) {
			auto pos = TilePos{change.getPos().getX(), change.getPos().getY()};
			plane_->tiles().forceSetBackgroundID(pos, change.getNewTile());
		}

		auto fluidData = tick.getFluidChangeData().asBytes();
		if (fluidData.size() != tick.getFluidChangePositions().size() * sizeof(FluidInTile)) {
			(warn
				<< "Bad fluid data! " << fluidData.size() << " bytes of fluid data for "
				<< tick.getFluidChangePositions().size() << " positions");
			return;
		}

		const Fluid::ID *fluidPtr = &fluidData.front();
		for (auto change: tick.getFluidChangePositions()) {
			auto pos = TilePos{change.getX(), change.getY()};
			plane_->fluids().setGridInTile(pos, fluidPtr);
			fluidPtr += sizeof(FluidInTile);
		}

		// Finally, send the player's current state
		sendPlayerState();
	} else {
		warn << "Received unknown message from server:";
		warn << r.toString().flatten().cStr();
	}
}

void MPGame::initInputHandler()
{
	std::vector<ActionSpec> actions;

	for (auto &mod: data_->mods_) {
		for (auto action: mod.mod_->actions_) {
			action.name = cat(mod.name(), "::", action.name);
			actions.push_back(std::move(action));
		}
	}

	inputHandler_.setActions(std::move(actions));
}

void MPGame::sendPlayerState()
{
	Entity *ent = player_.get();
	if (!ent) {
		info << "No entity!";
		return;
	}

	// TODO: Don't allocate every time

	capnp::MallocMessageBuilder mb;
	ent->serializeUpdates(plane_->getContext(), mb);

	kj::VectorOutputStream stream;
	capnp::writePackedMessage(stream, mb);
	auto arr = stream.getArray();

	auto builder = client_.builder();
	auto data = builder.initRoot<mp_proto::ClientToServer>().initUpdatePlayer(arr.size());
	memcpy(&data.asBytes().front(), &arr.front(), arr.size());
	client_.send(builder);
}

}
