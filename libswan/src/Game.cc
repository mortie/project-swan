#include "Game.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <math.h>
#include <time.h>
#include <memory>
#include <imgui/imgui.h>
#include <cygnet/gl.h>
#include <stb/stb_image_write.h>
#include <date/date.h>
#include <date/tz.h>

#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <kj/filesystem.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Clock.h"
#include "swan.capnp.h"

#include "traits/InventoryTrait.h"
#include "EntityCollectionImpl.h" // IWYU pragma: keep

namespace Swan {

static constexpr float TICK_DELTA = 1.0 / 20.0;

Game::Game(std::function<bool()> recompileMods):
	recompileMods_(std::move(recompileMods))
{
	const char *val = getenv("SWAN_DEBUG_KEYS");
	if (val && std::string_view(val) == "1") {
		debug_.showInputDebug = true;
	}
}

void Game::createWorld(
	std::string worldPath, const std::string &worldgen,
	uint32_t seed, std::span<std::string> modPaths)
{
	world_ = std::make_unique<World>(this, seed, modPaths);
	for (auto &mod: world_->mods_) {
		mod.mod_->start(*world_);
	}

	initInputHandler();

	world_->setWorldGen(worldgen);
	world_->setCurrentPlane(world_->addPlane());
	world_->spawnPlayer();
	hasSortedItems_ = false;
	worldPath_ = std::move(worldPath);
}

void Game::loadWorld(
	std::string worldPath, std::span<const std::string> modPaths)
{
	auto fs = kj::newDiskFilesystem();
	auto worldFile = fs->getCurrent().openFile(kj::Path::parse(worldPath));
	auto bytes = worldFile->readAllBytes();
	auto data = kj::ArrayInputStream(bytes);
	capnp::PackedMessageReader reader(data);

	world_ = std::make_unique<World>(this, 0, modPaths);
	for (auto &mod: world_->mods_) {
		mod.mod_->start(*world_);
	}

	initInputHandler();

	auto world = reader.getRoot<proto::World>();
	world_->deserialize(world);
	hasSortedItems_ = false;
	worldPath_ = std::move(worldPath);
}

void Game::onMouseMove(float x, float y)
{
	Vec2 pixPos{x, y};
	mousePos_ = (pixPos / cam_.size.as<float>()) * renderer_.winScale();
	pixPos -= uiCam_.size / 2;
	mouseUIPos_ = (pixPos / uiCam_.size / uiCam_.zoom * 2) * renderer_.winScale();
	gui_.onMouseMove(mouseUIPos_);
	hasMouseMoved_ = true;
}

void Game::onScrollWheel(double dy)
{
	didScroll_ += dy;
}

Vec2 Game::getMousePos()
{
	return (getMouseScreenPos() * 2 - renderer_.winScale()) / cam_.zoom + cam_.pos;
}

TilePos Game::getMouseTile()
{
	auto pos = (getMouseScreenPos() * 2 - renderer_.winScale()) / cam_.zoom + cam_.pos;

	return TilePos{(int)floor(pos.x), (int)floor(pos.y)};
}

void Game::drawDebugMenu()
{
	ImGui::Text(
		"Position: x=%d y=%d",
		int(round(world_->player_->pos.x)),
		int(round(world_->player_->pos.y)));

	ImGui::Text("World seed: %u", world_->seed());

	ImGui::Checkbox("Draw collision boxes", &debug_.drawCollisionBoxes);
	ImGui::Checkbox("Draw chunk boundaries", &debug_.drawChunkBoundaries);
	ImGui::Checkbox("Draw world ticks", &debug_.drawWorldTicks);

	bool prevEnableVSync = enableVSync_;
	ImGui::Checkbox("Enable VSync", &enableVSync_);
	if (enableVSync_ && !prevEnableVSync) {
		glfwSwapInterval(1);
	}
	else if (!enableVSync_ && prevEnableVSync) {
		glfwSwapInterval(0);
	}

	ImGui::Checkbox("Show fluid particles", &debug_.fluidParticleLocations);
	ImGui::Checkbox("Disable shadows", &debug_.disableShadows);

	ImGui::Checkbox("Hand-break any tile", &debug_.handBreakAny);
	ImGui::Checkbox("God mode", &debug_.godMode);
	ImGui::Checkbox("Infinite items", &debug_.infiniteItems);

	ImGui::Checkbox("Individually serialize entities", &debug_.outputEntityProto);

	ImGui::Checkbox("Show input debug menu", &debug_.showInputDebug);

	if (ImGui::Button("Save World")) {
		triggerSave_ = true;
	}

	if (ImGui::Button("Screenshot")) {
		std::filesystem::create_directories("screenshots");
		auto now = std::chrono::floor<std::chrono::seconds>(
			std::chrono::system_clock::now());
		auto nowZoned = date::make_zoned(date::current_zone(), now);
		auto path = date::format({}, "screenshots/%F %T.png", nowZoned);
		screenshot(path.c_str());
	}

	if (!FrameRecorder::isAvailable()) {
		ImGui::Text("Screen recording unavailable");
	}
	else if (frameRecorder_) {
		if (ImGui::Button("End recording")) {
			frameRecorder_->end();
			frameRecorder_.reset();
			fixedDeltaTime_.reset();
		}
	}
	else {
		if (ImGui::Button("Begin recording")) {
			frameRecorder_.emplace();

			int fps = 60;
			int width = 1920;
			int height = int((float(width) / cam_.size.x) * cam_.size.y);

			fixedDeltaTime_ = 1.0 / fps;
			fpsLimit_ = fps;
			if (!frameRecorder_->begin(width, height, fps, "rec.mp4")) {
				info << "Recording failed!";
				frameRecorder_.reset();
				fixedDeltaTime_.reset();
			}
		}
	}

	ImGui::SliderFloat(
		"Time scale", &timeScale_, 0, 3.0, "%.03f",
		ImGuiSliderFlags_Logarithmic);
	if (ImGui::BeginPopupContextItem("Time scale menu")) {
		if (ImGui::MenuItem("Reset")) {
			timeScale_ = 1.0;
		}
		ImGui::EndPopup();
	}

	float oldVolume = soundPlayer_.volume();
	float volume = oldVolume;
	ImGui::SliderFloat(
		"Volume", &volume, 0, 1.0, "%.03f",
		ImGuiSliderFlags_Logarithmic);
	if (ImGui::BeginPopupContextItem("Volume menu")) {
		if (ImGui::MenuItem("Reset")) {
			volume = 1.0;
		}
		ImGui::EndPopup();
	}

	if (volume != oldVolume) {
		soundPlayer_.volume(volume);
	}

	ImGui::SliderFloat(
		"UI scale", &uiCam_.zoom, 0, 1.0, "%.03f",
		ImGuiSliderFlags_Logarithmic);
	if (ImGui::BeginPopupContextItem("UI scale menu")) {
		if (ImGui::MenuItem("Reset")) {
			uiCam_.zoom = 1.0 / 16;
		}
		ImGui::EndPopup();
	}

	std::initializer_list<std::pair<const char *, int>> fpsPresets = {
		{"30", 30},
		{"60", 60},
		{"75", 75},
		{"90", 00},
		{"100", 100},
		{"120", 120},
		{"144", 144},
		{"240", 240},
		{"360", 360},
	};

	bool fpsLimitChanged = ImGui::SliderFloat(
		"FPS limit", &fpsLimit_, 10, 240, "%.00f", 0);
	if (fpsLimitChanged) {
		fpsLimit_ = int(fpsLimit_);
	}

	if (ImGui::BeginPopupContextItem("FPS limit menu")) {
		if (ImGui::MenuItem("Disable")) {
			fpsLimit_ = 0;
		}
		for (auto &[name, num]: fpsPresets) {
			if (ImGui::MenuItem(name)) {
				fpsLimit_ = num;
			}
		}
		ImGui::EndPopup();
	}

	if (fpsLimit_ < 10) {
		fpsLimit_ = 0;
	}

	auto &tile = world_->currentPlane().tiles().get(getMouseTile());
	auto mouseTile = getMouseTile();
	ImGui::Text("Tile: %s (%d, %d)\n", tile.name.c_str(), mouseTile.x, mouseTile.y);

	if (!hasSortedItems_) {
		sortedItems_.clear();
		sortedItems_.reserve(world_->items_.size());
		for (auto &item: world_->items_) {
			sortedItems_.push_back(&item);
		}
		std::sort(sortedItems_.begin(), sortedItems_.end(), [](Item *a, Item *b) {
			return a->name < b->name;
		});
	}

	ImGui::Text("Give Item:");
	ImGui::BeginChild("Give Item", {0, 200});
	for (auto item: sortedItems_) {
		if (item->hidden) {
			continue;
		}

		if (ImGui::Button(item->name.c_str())) {
			auto *inventory = world_->playerRef_.trait<InventoryTrait>();
			ItemStack stack(item, 1);

			info << "Giving player " << stack.count() << ' ' << item->name;
			inventory->insert(stack);
		}
	}
	ImGui::EndChild();
}

void Game::drawPerfMenu()
{
	auto &fluids = world_->currentPlane().fluids();
	ImGui::Text("FPS: %d", perf_.fps);
	ImGui::Text("TPS: %d", perf_.tps);
	ImGui::Text(
		"Fluid updates: %d, particles: %d",
		fluids.numUpdates(), fluids.numParticles());

	ImGui::Separator();

	ImGui::Text("Entity update: %.2fms avg / %.2fms max",
		perf_.entityUpdateTime.avgMs, perf_.entityTickTime.maxMs);
	ImGui::Text("Entity tick:   %.2fms avg / %.2fms max",
		perf_.entityTickTime.avgMs, perf_.entityTickTime.maxMs);
	ImGui::Text("Tile tick:     %.2fms avg / %.2fms max",
		perf_.tileTickTime.avgMs, perf_.tileTickTime.maxMs);
	ImGui::Text("Fluid update:  %.2fms avg / %.2fms max",
		perf_.fluidUpdateTime.avgMs, perf_.fluidUpdateTime.maxMs);
	ImGui::Text("Fluid tick:    %.2fms avg / %.2fms max",
		perf_.fluidTickTime.avgMs, perf_.fluidTickTime.maxMs);
	ImGui::Text("World tick:    %.2fms avg / %.2f max",
		perf_.worldTickTime.avgMs, perf_.worldTickTime.maxMs);
	ImGui::Text("Chunk count:   %zu (active: %zu)",
		world_->currentPlane().getChunkCount(),
		world_->currentPlane().getActiveChunkCount());
	ImGui::Text("Chunk memory:  %.02f MiB",
		world_->currentPlane().getChunkDataMemUsage() / double(1024 * 1024));
}

void Game::draw()
{
	auto now = std::chrono::duration<double>(
		std::chrono::steady_clock::now().time_since_epoch()).count();
	frameCount_ += 1;

	if (now - fpsUpdateTime_ >= 1) {
		perf_.fps = frameCount_;
		perf_.tps = tickCount_;
		frameCount_ = 0;
		tickCount_ = 0;
		fpsUpdateTime_ += 1;
	}

	if (debug_.show) {
		ImGui::Begin(
			"Debug Menu", &debug_.show,
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		drawDebugMenu();
		ImGui::End();
	}

	for (size_t i = 0; i < debugEntities_.size(); ++i) {
		auto &ref = debugEntities_[i];
		auto ent = ref.get();
		if (!ent) {
			continue;
		}

		ImGui::Begin(
			cat("Entity debug window [", i, "]").c_str(), nullptr,
			ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text(
			"Debug for %s %" PRIu64,
			ref.collection()->name().c_str(), ref.id());
		ent->drawDebug(world_->currentPlane().getContext());
		ImGui::End();
	}

	int rightPanelY = 0;

	if (perf_.show) {
		ImGui::Begin(
			"Perf Menu", &perf_.show,
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing);
		ImVec2 pos(
			ImGui::GetIO().DisplaySize.x - ImGui::GetWindowWidth(),
			rightPanelY);
		ImGui::SetWindowPos(pos, ImGuiCond_Always);
		rightPanelY += ImGui::GetWindowHeight();
		drawPerfMenu();
		ImGui::End();
	}

	if (debug_.showInputDebug) {
		ImGui::SetNextWindowSizeConstraints(
			ImVec2(250, 0),
			ImVec2(FLT_MAX, FLT_MAX));
		ImGui::Begin(
			"Input Debug Menu", &debug_.showInputDebug,
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing);
		ImVec2 pos(
			ImGui::GetIO().DisplaySize.x - ImGui::GetWindowWidth(),
			rightPanelY);
		ImGui::SetWindowPos(pos, ImGuiCond_Always);
		rightPanelY += ImGui::GetWindowHeight();
		inputHandler_.drawDebug();
		ImGui::End();
	}

	if (popupMessage_ != "") {
		ImGui::Begin(
			"Popup", nullptr,
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoTitleBar);
		ImGui::SetWindowPos(
			ImVec2(
				ImGui::GetIO().DisplaySize.x / 2 - ImGui::GetWindowWidth() / 2,
				ImGui::GetIO().DisplaySize.y / 2 - ImGui::GetWindowHeight() / 2),
			ImGuiCond_Always);
		ImGui::Text("%s", popupMessage_.c_str());
		ImGui::End();
	}

	renderer_.clear();
	renderer_.setBackgroundColor(world_->backgroundColor());
	world_->draw(renderer_);
	gui_.endFrame();
}

void Game::render()
{
	renderer_.render(cam_);
	renderer_.renderUI(uiCam_);

	if (frameRecorder_) {
		renderer_.drawUIRect({
			.pos = mouseUIPos_.add(-0.05, -0.05),
			.size = {0.1, 0.1},
			.outline = {1, 1, 1, 1},
			.fill = {0, 0, 0, 1},
		});
		auto size = frameRecorder_->size();
		frameRecorder_->beginFrame();
		renderer_.render(cam_.withSize(size), {.vflip = true});
		renderer_.renderUI(uiCam_.withSize(size), {.vflip = true});
		frameRecorder_->endFrame();
	}
}

void Game::screenshot(const char *path, int w, int h)
{
	if (w < 0) {
		w = 1920;
	} else if (w < 8) {
		w = 8;
	}

	if (h < 0) {
		h = 1080;
	} else if (h < 8) {
		h = 8;
	}

	info << "Writing " << w << 'x' << h << " screenshot to '" << path << "'...";

	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	Cygnet::glCheck();
	SWAN_DEFER(glDeleteFramebuffers(1, &fbo));

	GLuint fboTex;
	glGenTextures(1, &fboTex);
	Cygnet::glCheck();
	SWAN_DEFER(glDeleteTextures(1, &fboTex));

	glBindTexture(GL_TEXTURE_2D, fboTex);
	Cygnet::glCheck();
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	Cygnet::glCheck();

	GLint screenFBO;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &screenFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	Cygnet::glCheck();
	SWAN_DEFER(glBindFramebuffer(GL_FRAMEBUFFER, screenFBO));

	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		fboTex, 0);
	Cygnet::glCheck();

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glViewport(0, 0, w, h);
	Cygnet::glCheck();
	SWAN_DEFER(glViewport(viewport[0], viewport[1], viewport[2], viewport[3]));

	auto cam = cam_.withSize({w, h});
	renderer_.render(cam, {.vflip = true});

	auto pixels = std::make_unique<unsigned char[]>(w * h * 4);
	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.get());
	Cygnet::glCheck();

	if (!stbi_write_png(path, w, h, 4, pixels.get(), w * 4)) {
		warn << "Writing screenshot failed";
	}
}

void Game::update(float dt)
{
	inputHandler_.beginFrame();

	if (*uiActivateAction_) {
		gui_.triggerActivate();
	}

	perf_.updateCount += 1;
	if (perf_.updateCount >= 60) {
		perf_.entityUpdateTime.capture(perf_.updateCount);
		perf_.fluidUpdateTime.capture(perf_.updateCount);
		perf_.worldTickTime.capture(perf_.updateCount);
		perf_.updateCount = 0;
	}

	// Zoom the window using the scroll wheel
	cam_.zoom += (float)didScroll_ * 0.05f * cam_.zoom;

	// Zoom using the controller
	if (uiModAction_->direction() > 0) {
		cam_.zoom += -uiCameraZoomAction_->activation * cam_.zoom * dt * 2;
	}

	float zoomLim = debug_.godMode ? 0.002 : 0.0175;
	if (cam_.zoom > 1) {
		cam_.zoom = 1;
	}
	else if (cam_.zoom < zoomLim) {
		cam_.zoom = zoomLim;
	}

	renderer_.setCull({
		.pos = cam_.pos,
		.size = {1 / cam_.zoom, 1 / cam_.zoom},
	});

	if (*entityDebugMenuAction_) {
		bool found = false;

		auto tile = getMouseTile();
		auto ents = world_->currentPlane().entities().getInTile(tile);
		for (auto &ent: ents) {
			debugEntities_.push_back(ent.ref);
			found = true;
		}

		auto ent = world_->currentPlane().entities().getTileEntity(tile);
		if (ent) {
			debugEntities_.push_back(ent);
			found = true;
		}

		if (!found) {
			debugEntities_.clear();
		}
	}

	if (*debugMenuAction_) {
		debug_.show = !debug_.show;
	}

	if (*perfMenuAction_) {
		perf_.show = !perf_.show;
	}

	if (popupMessageTimer_ > 0) {
		popupMessageTimer_ -= dt;
		if (popupMessageTimer_ <= 0) {
			popupMessage_ = "";
		}
	}

	if (triggerReload_ == 1) {
		triggerReload_ = false;
		if (reload()) {
			popupMessage_ = "";
		} else {
			popupMessage_ = "Reload failed!";
			popupMessageTimer_ = 2;
		}
	} else if (*reloadModsAction_) {
		popupMessage_ = "Reloading...";
		popupMessageTimer_ = 1;
		triggerReload_ = 3;
	} else if (triggerReload_ > 0) {
		triggerReload_ -= 1;
	}

	if (*regenWorldAction_) {
		popupMessage_ = "Regenerating...";
		popupMessageTimer_ = 0.5;
		world_->currentPlane().regenerate();
	}

	renderer_.update(dt);
	world_->update(dt);

	soundPlayer_.setCenter(cam_.pos.x, cam_.pos.y);
	didScroll_ = 0;
	hasMouseMoved_ = false;
	inputHandler_.endFrame();

	tickAcc_ += dt;
	if (tickAcc_ > 2) {
		warn << "Skipping ticks due to system overload!";
		tickAcc_ = TICK_DELTA;
	}

	if (tickInProgress_) {
		tickDeadline_.reset();
		if (world_->tick(TICK_DELTA, tickDeadline_)) {
			tickInProgress_ = false;
		}
	}
	else if (tickAcc_ >= TICK_DELTA) {
		tick();
		tickAcc_ -= TICK_DELTA;
	}
}

void Game::tick()
{
	tickCount_ += 1;

	if (triggerSave_) {
		save();
		triggerSave_ = false;
	}

	perf_.tickCount += 1;
	if (perf_.tickCount >= 20) {
		perf_.entityTickTime.capture(perf_.tickCount);
		perf_.tileTickTime.capture(perf_.tickCount);
		perf_.fluidTickTime.capture(perf_.tickCount);
		perf_.tickCount = 0;
	}

	if (fpsLimit_ > 0) {
		// Allocate half a frame to a tick
		tickDeadline_ = RTDeadline(0.5 / fpsLimit_);
	}
	else {
		// If there's no FPS limit, allocate 2ms
		tickDeadline_ = RTDeadline(2.0 / 1000.0);
	}

	tickInProgress_ = true;
	if (world_->tick(TICK_DELTA, tickDeadline_)) {
		tickInProgress_ = false;
	}
}

void Game::save()
{
	if (tickInProgress_) {
		info << "Completing current tick...";
		if (world_->tick(TICK_DELTA, RTDeadline(2))) {
			tickInProgress_ = false;
		} else {
			warn << "Failed to complete tick in 2 seconds!";
		}
	}

	info << "Serializing world...";
	capnp::MallocMessageBuilder mb;
	auto world = mb.initRoot<proto::World>();
	world_->serialize(world);

	auto fs = kj::newDiskFilesystem();
	auto &dir = fs->getCurrent();

	info << "Writing to " << worldPath_ << "...";
	auto replacer = dir.replaceFile(
		kj::Path::parse(worldPath_), kj::WriteMode::CREATE | kj::WriteMode::MODIFY);
	auto appender = kj::newFileAppender(replacer->get().clone());
	capnp::writePackedMessage(*appender, mb);

	info << "Done!";
	replacer->commit();
}

bool Game::reload()
{
	RTClock startTime;

	if (!recompileMods_()) {
		warn << "Failed to recompile mods!";
		return false;
	}

	std::vector<std::string> mods;
	for (auto &mod: world_->mods_) {
		mods.push_back(mod.path_);
	}

	save();
	soundPlayer_.flush();
	world_.reset();
	loadWorld(worldPath_, mods);
	debugEntities_.clear();

	info << "Reloaded in " << startTime.duration() << " seconds.";
	return true;
}

void Game::initInputHandler()
{
	auto actions = std::move(world_->actions_);

	actions.push_back({
		.name = "@::entity-debug-menu",
		.kind = ActionKind::ONESHOT,
		.defaultInputs = {"key:F1"},
	});

	actions.push_back({
		.name = "@::debug-menu",
		.kind = ActionKind::ONESHOT,
		.defaultInputs = {"key:F3"},
	});

	actions.push_back({
		.name = "@::perf-menu",
		.kind = ActionKind::ONESHOT,
		.defaultInputs = {"key:F4"},
	});

	actions.push_back({
		.name = "@::reload-mods",
		.kind = ActionKind::ONESHOT,
		.defaultInputs = {"key:F5"},
	});

	actions.push_back({
		.name = "@::regen-world",
		.kind = ActionKind::ONESHOT,
		.defaultInputs = {"key:F6"},
	});

	actions.push_back({
		.name = "@::ui-activate",
		.kind = ActionKind::ONESHOT,
		.defaultInputs = {"mouse:LEFT", "button:A"},
	});

	actions.push_back({
		.name = "@::ui-mod",
		.kind = ActionKind::AXIS,
		.defaultInputs = {"axis:LEFT_TRIGGER"},
	});

	actions.push_back({
		.name = "@::ui-camera-zoom",
		.kind = ActionKind::AXIS,
		.defaultInputs = {"axis:RIGHT_Y"},
	});

	inputHandler_.setActions(std::move(actions));
	entityDebugMenuAction_ = inputHandler_.action("@::entity-debug-menu");
	debugMenuAction_ = inputHandler_.action("@::debug-menu");
	perfMenuAction_ = inputHandler_.action("@::perf-menu");
	reloadModsAction_ = inputHandler_.action("@::reload-mods");
	regenWorldAction_ = inputHandler_.action("@::regen-world");
	uiActivateAction_ = inputHandler_.action("@::ui-activate");
	uiModAction_ = inputHandler_.action("@::ui-mod");
	uiCameraZoomAction_ = inputHandler_.action("@::ui-camera-zoom");
}

}
