#include "GameServer.h"

#include "World.h"
#include "Game.h"
#include "capnp/message.h"
#include "capnp/serialize-packed.h"
#include "EntityCollectionImpl.h"

namespace Swan {

void GameServer::onTileChange(WorldPlane::ID plane, TilePos pos, Tile::ID newID)
{
	if (clients_.empty()) {
		return;
	}

	planeChanges(plane).tileChanges.push_back({
		.pos = pos,
		.newID = newID,
	});
}

void GameServer::onBackgroundTileChange(WorldPlane::ID plane, TilePos pos, Tile::ID newID)
{
	if (clients_.empty()) {
		return;
	}

	planeChanges(plane).backgroundChanges.push_back({
		.pos = pos,
		.newID = newID,
	});
}

void GameServer::tick(float dt)
{
	server_.tick(dt);

	for (auto &plane: world_->planes()) {
		broadcastTickUpdateForPlane(*plane.plane);
	}

	mp_proto::ClientToServer::Reader r;
	const MPServer::ClientInfo *client;
	while ((client = server_.receive(r))) {
		onMessageFromClient(*client, r);
	}
}

void GameServer::onMessageFromClient(
	const MPServer::ClientInfo &client,
	mp_proto::ClientToServer::Reader &r)
{
	if (r.isHello()) {
		if (!r.getHello().getRequestWorld()) {
			// Nothing to do here; the client didn't request world data,
			// so we won't do the world sync
			return;
		}

		auto player = game_->onPlayerConnected(r.getHello().getIdentifier().cStr());
		auto &plane = world_->getPlane(player.plane);

		auto root = server_.builder();
		auto sync = root.initWorldSync();

		auto modIDs = sync.initModIDs(modIDs_.size());
		for (size_t i = 0; auto &id: modIDs_) {
			modIDs.set(i, id);
		}

		auto namesByID = sync.initNamesByID(world_->data().namesByID_.size());
		for (size_t i = 0; auto &name: world_->data().namesByID_) {
			namesByID.set(i++, name);
		}

		sync.setCurrentPlaneIndex(player.plane);
		plane.plane->serialize(sync.initCurrentPlane());
		sync.getCurrentPlane().setWorldGen(plane.worldGen);
		sync.setWorldSeed(world_->seed());
		player.ref.serialize(sync.initPlayerRef());

		server_.send(client.id, root);

		clients_.push_back(ConnectedClient {
			.info = client,
			.plane = player.plane,
			.ref = player.ref,
		});
	} else if (r.isQuit()) {
		info << "Client " << client.identifier << " disconnected.";

		for (size_t i = 0; i < clients_.size(); ++i) {
			if (clients_[i].info.id != client.id) {
				continue;
			}

			clients_[i] = std::move(clients_.back());
			clients_.pop_back();
			break;
		}
	} else if (r.isUpdatePlayer()) {
		auto &c = *std::find_if(clients_.begin(), clients_.end(), [&](auto &c) {
			return c.info.id == client.id;
		});
		Entity *ent = c.ref.get();
		if (!ent) {
			warn << "Update from player without entity!";
			return;
		}

		auto &plane = *world_->getPlane(c.plane).plane;
		kj::ArrayInputStream stream(r.getUpdatePlayer());
		capnp::PackedMessageReader reader(stream);
		auto override = plane.entities().overrideCurrentEntity(c.ref);
		ent->deserializeUpdates(plane.getContext(), reader);
	} else {
		info << "Received unknown message from client '" << client.identifier << ":";
		info << r.toString().flatten().cStr();
	}
}

void GameServer::broadcastTickUpdateForPlane(WorldPlane &plane)
{
	bool hasClientOnPlane = false;
	for (auto &client: clients_) {
		if (client.plane == plane.id_) {
			hasClientOnPlane = true;
			break;
		}
	}
	if (!hasClientOnPlane) {
		return;
	}

	auto root = server_.builder();
	auto tick = root.initTick();

	Ctx ctx = plane.getContext();

	size_t updatedCollectionsCount = 0;
	for (auto &coll: world_->currentPlane().entities().collections()) {
		if (coll->hasUpdated()) {
			updatedCollectionsCount += 1;
		}
	}

	auto updatedCollections = tick.initUpdatedEntityCollections(updatedCollectionsCount);
	for (size_t id = 0, idx = 0; auto &coll: world_->currentPlane().entities().collections()) {
		if (!coll->hasUpdated()) {
			idx += 1;
			continue;
		}

		coll->serializeUpdates(ctx, updatedCollections[id]);
		updatedCollections[id].setIndex(idx);
		id += 1;
		idx += 1;
	}

	{ // Serialize world gen data
		stream_.clear();
		capnp::MallocMessageBuilder builder(scratch_);
		plane.worldGen_->serialize(ctx, builder);
		capnp::writePackedMessage(stream_, builder);
		auto arr = stream_.getArray();
		auto data = tick.initWorldGenData(arr.size());
		memcpy(&data.front(), &arr.front(), arr.size());
	}

	PlaneChanges &changes = planeChanges(plane.id_);

	if (!changes.tileChanges.empty()) {
		auto arr = tick.initTileChanges(changes.tileChanges.size());
		size_t len = changes.tileChanges.size();
		for (size_t i = 0; i < len; ++i) {
			auto changeBuilder = arr[i];
			auto &change = changes.tileChanges[i];
			changeBuilder.setNewTile(change.newID);
			auto pos = changeBuilder.initPos();
			pos.setX(change.pos.x);
			pos.setY(change.pos.y);
		}
		changes.tileChanges.clear();
	}

	if (!changes.backgroundChanges.empty()) {
		auto arr = tick.initBackgroundChanges(changes.backgroundChanges.size());
		size_t len = changes.backgroundChanges.size();
		for (size_t i = 0; i < len; ++i) {
			auto changeBuilder = arr[i];
			auto &change = changes.backgroundChanges[i];
			changeBuilder.setNewTile(change.newID);
			auto pos = changeBuilder.initPos();
			pos.setX(change.pos.x);
			pos.setY(change.pos.y);
		}
		changes.backgroundChanges.clear();
	}

	const auto &changedFluidTiles = plane.fluids().getChangedTiles();
	auto fluidData = tick.initFluidChangeData(changedFluidTiles.size() * sizeof(FluidInTile));
	auto fluidPositions = tick.initFluidChangePositions(changedFluidTiles.size());
	auto fluidPtr = &fluidData.asBytes().front();
	for (size_t i = 0; auto &pos: changedFluidTiles) {
		plane.fluids().getGridInTile(pos, &fluidPtr[i * sizeof(FluidInTile)]);
		fluidPositions[i].setX(pos.x);
		fluidPositions[i].setY(pos.y);
		i += 1;
	}

	broadcastToPlane(plane.id_, root);
}

void GameServer::broadcastToPlane(
	WorldPlane::ID plane,
	const mp_proto::ServerToClient::Builder &root)
{
	size_t numOnCorrectPlane = 0;
	for (auto &client: clients_) {
		if (client.plane == plane) {
			numOnCorrectPlane += 1;
		}
	}

	if (numOnCorrectPlane == 0) {
		// Don't bother, we have nobody on that plane
		return;
	} else if (numOnCorrectPlane == clients_.size()) {
		// Just broadcast, everyone is on the right plane
		server_.broadcast(root);
		return;
	}

	for (auto &client: clients_) {
		// TODO: This should be optimized,
		// we're encoding for every client now
		server_.send(client.info.id, root);
	}
}

}
