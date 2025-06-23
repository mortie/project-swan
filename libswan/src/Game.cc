#include "Game.h"

#include <algorithm>
#include <cmath>
#include <math.h>
#include <time.h>
#include <memory>
#include <imgui/imgui.h>

#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <kj/filesystem.h>

#include "Clock.h"
#include "swan.capnp.h"

#include "traits/InventoryTrait.h"
#include "EntityCollectionImpl.h" // IWYU pragma: keep

namespace Swan {

void Game::createWorld(
	const std::string &worldgen, std::span<std::string> modPaths)
{
	world_ = std::make_unique<World>(this, time(NULL), modPaths);
	for (auto &mod: world_->mods_) {
		mod.mod_->start(*world_);
	}

	world_->setWorldGen(worldgen);
	world_->setCurrentPlane(world_->addPlane());
	world_->spawnPlayer();
	hasSortedItems_ = false;
}

void Game::loadWorld(
	kj::BufferedInputStream &is, std::span<const std::string> modPaths)
{
	capnp::PackedMessageReader reader(is);

	world_ = std::make_unique<World>(this, time(NULL), modPaths);
	for (auto &mod: world_->mods_) {
		mod.mod_->start(*world_);
	}

	auto world = reader.getRoot<proto::World>();
	world_->deserialize(world);
	hasSortedItems_ = false;
}

void Game::onKeyDown(int scancode, int key)
{
	pressedKeys_[scancode] = true;
	didPressKeys_[scancode] = true;
	if (key >= 0) {
		pressedLiteralKeys_[key] = true;
		didPressLiteralKeys_[key] = true;
	}
}

void Game::onKeyUp(int scancode, int key)
{
	pressedKeys_[scancode] = false;
	if (key >= 0) {
		pressedLiteralKeys_[key] = false;
	}
}

void Game::onMouseMove(float x, float y)
{
	Vec2 pixPos{x, y};
	mousePos_ = (pixPos / cam_.size.as<float>()) * renderer_.winScale();
	pixPos -= uiCam_.size / 2;
	mouseUIPos_ = (pixPos / uiCam_.size / uiCam_.zoom * 2) * renderer_.winScale();
}

void Game::onMouseDown(int button)
{
	pressedButtons_[button] = true;
	didPressButtons_[button] = true;
}

void Game::onMouseUp(int button)
{
	pressedButtons_[button] = false;
	didReleaseButtons_[button] = true;
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
	ImGui::Begin(
		"Debug Menu", &debug_.show,
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing);
	ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);

	ImGui::Text(
		"Position: x=%d y=%d",
		int(round(world_->player_->pos.x)),
		int(round(world_->player_->pos.y)));

	ImGui::Checkbox("Draw collision boxes", &debug_.drawCollisionBoxes);
	ImGui::Checkbox("Draw chunk boundaries", &debug_.drawChunkBoundaries);

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

	ImGui::Checkbox("Individually serialize entities", &debug_.outputEntityProto);
	if (ImGui::Button("Save")) {
		triggerSave_ = true;
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

	ImGui::SliderFloat(
		"FPS limit", &fpsLimit_, 10, 360.0, "%.03f", 0);
	if (ImGui::BeginPopupContextItem("FPS limit menu")) {
		if (ImGui::MenuItem("Disable")) {
			fpsLimit_ = 0;
		}
		else if (ImGui::MenuItem("30")) {
			fpsLimit_ = 30;
		}
		else if (ImGui::MenuItem("60")) {
			fpsLimit_ = 60;
		}
		else if (ImGui::MenuItem("90")) {
			fpsLimit_ = 90;
		}
		else if (ImGui::MenuItem("120")) {
			fpsLimit_ = 120;
		}
		else if (ImGui::MenuItem("144")) {
			fpsLimit_ = 144;
		}
		ImGui::EndPopup();
	}

	if (fpsLimit_ < 10) {
		fpsLimit_ = 0;
	}

	auto &tile = world_->currentPlane().tiles().get(getMouseTile());
	ImGui::Text("Tile: %s\n", tile.name.c_str());

	if (!hasSortedItems_) {
		sortedItems_.clear();
		sortedItems_.reserve(world_->items_.size());
		for (auto &[name, item]: world_->items_) {
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

	ImGui::End();
}

void Game::drawPerfMenu()
{
	ImGui::Begin(
		"Perf Menu", &perf_.show,
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing);
	ImGui::SetWindowPos(
		ImVec2(ImGui::GetIO().DisplaySize.x - ImGui::GetWindowWidth(), 0),
		ImGuiCond_Always);

	auto &fluids = world_->currentPlane().fluids();
	ImGui::Text("FPS: %d", perf_.fps);
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
	ImGui::Text("Chunk count:   %zu (active: %zu)",
		world_->currentPlane().getChunkCount(),
		world_->currentPlane().getActiveChunkCount());
	ImGui::Text("Chunk memory:  %.02f MiB",
		world_->currentPlane().getChunkDataMemUsage() / double(1024 * 1024));

	ImGui::End();
}

void Game::draw()
{
	auto now = std::chrono::steady_clock::now();

	if (now > fpsUpdateTime_ + std::chrono::milliseconds(200) && frameAcc_ > 0) {
		float avgFrameTime = std::chrono::duration_cast<std::chrono::duration<float>>(frameTimeAcc_).count() / frameAcc_;
		perf_.fps = std::round(1.0f / avgFrameTime);
		frameAcc_ = 0;
		frameTimeAcc_ = {};
		fpsUpdateTime_ = now;
	}
	else {
		frameTimeAcc_ += now - prevTime_;
		frameAcc_ += 1;
	}
	prevTime_ = now;

	if (debug_.show) {
		drawDebugMenu();
	}

	if (perf_.show) {
		drawPerfMenu();
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

	world_->draw(renderer_);
}

void Game::render()
{
	renderer_.setBackgroundColor(world_->backgroundColor());
	renderer_.render(cam_);
	renderer_.renderUI(uiCam_);

	if (frameRecorder_) {
		renderer_.drawUIRect(Cygnet::RenderLayer::FOREGROUND, {
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

	renderer_.clear();
}

void Game::update(float dt)
{
	perf_.updateCount += 1;
	if (perf_.updateCount >= 60) {
		perf_.entityUpdateTime.capture(perf_.updateCount);
		perf_.fluidUpdateTime.capture(perf_.updateCount);
		perf_.updateCount = 0;
	}

	float zoomLim = debug_.godMode ? 0.002 : 0.0175;
	// Zoom the window using the scroll wheel
	cam_.zoom += (float)wasWheelScrolled() * 0.05f * cam_.zoom;
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

	if (wasLiteralKeyPressed(GLFW_KEY_F3)) {
		debug_.show = !debug_.show;
	}

	if (wasLiteralKeyPressed(GLFW_KEY_F4)) {
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
	} else if (wasLiteralKeyPressed(GLFW_KEY_F5)) {
		popupMessage_ = "Reloading...";
		popupMessageTimer_ = 1;
		triggerReload_ = 3;
	} else if (triggerReload_ > 0) {
		triggerReload_ -= 1;
	}

	if (wasLiteralKeyPressed(GLFW_KEY_F6)) {
		popupMessage_ = "Regenerating...";
		popupMessageTimer_ = 0.5;
		world_->currentPlane().regenerate();
	}

	renderer_.update(dt);
	world_->update(dt);

	soundPlayer_.setCenter(cam_.pos.x, cam_.pos.y);

	didScroll_ = 0;
	didPressKeys_.reset();
	didReleaseKeys_.reset();
	didPressLiteralKeys_.reset();
	didReleaseLiteralKeys_.reset();
	didPressButtons_.reset();
	didReleaseButtons_.reset();

	// Continue running the ongoing tick if necessary
	if (tickInProgress_) {
		tickDeadline_.reset();
		if (world_->tick(tickDT_, tickDeadline_)) {
			tickInProgress_ = false;
		}
	} else if (pendingTick_) {
		tick(tickDT_);
	}
}

void Game::tick(float dt)
{
	if (triggerSave_) {
		save();
		triggerSave_ = false;
	}

	if (tickInProgress_) {
		if (pendingTick_) {
			warn << "Skipping tick because previous tick is still not done!";
		} else {
			pendingTick_ = true;
		}
		return;
	}

	perf_.tickCount += 1;
	if (perf_.tickCount >= 20) {
		perf_.entityTickTime.capture(perf_.tickCount);
		perf_.tileTickTime.capture(perf_.tickCount);
		perf_.fluidTickTime.capture(perf_.tickCount);
		perf_.tickCount = 0;
	}

	if (fpsLimit_ > 0) {
		// Allocate a quarter of a frame to a tick
		tickDeadline_ = RTDeadline(0.25 / fpsLimit_);
	} else {
		// If there's no FPS limit, allocate 2ms
		tickDeadline_ = RTDeadline(2.0 / 1000.0);
	}

	pendingTick_ = false;
	tickInProgress_ = true;
	tickDT_ = dt;
	if (world_->tick(tickDT_, tickDeadline_)) {
		tickInProgress_ = false;
		pendingTick_ = false;
	}
}

void Game::save()
{
	if (tickInProgress_) {
		info << "Completing current tick...";
		if (world_->tick(tickDT_, RTDeadline(2))) {
			tickInProgress_ = false;
			pendingTick_ = false;
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

	info << "Writing to world.swan...";
	auto replacer = dir.replaceFile(
		kj::Path("world.swan"), kj::WriteMode::CREATE | kj::WriteMode::MODIFY);
	auto appender = kj::newFileAppender(replacer->get().clone());
	capnp::writePackedMessage(*appender, mb);

	info << "Done!";
	replacer->commit();
}

bool Game::reload()
{
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

	auto fs = kj::newDiskFilesystem();
	auto worldFile = fs->getCurrent().openFile(kj::Path("world.swan"));
	auto bytes = worldFile->readAllBytes();
	auto data = kj::ArrayInputStream(bytes);
	loadWorld(data, mods);
	return true;
}

}
