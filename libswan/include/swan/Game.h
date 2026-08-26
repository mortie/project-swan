#pragma once

#include "Clock.h"
#include <chrono>
#include <string>
#include <span>
#include <cygnet/Renderer.h>
#include <cygnet/Gui.h>
#include <cygnet/TextCache.h>
#include <cygnet/util.h>

#include <kj/io.h>
#include <vector>
#include <swan/HashMap.h>

#include "Command.h"
#include "GameIO.h"
#include "InputHandler.h"
#include "common.h"
#include "World.h"
#include "SoundPlayer.h"
#include "FrameRecorder.h"

namespace Swan {

class GameServer;

class Game: public GameIO {
public:
	Game(std::function<bool()> recompileMods, HashMap<ModInfo> mods);
	~Game();

	void createWorld(
		std::string worldPath, const std::string &worldgen,
		uint32_t seed);

	void loadWorld(std::string worldPath);

	void onMouseMove(float x, float y) override;
	void onScrollWheel(float dy) override;
	void onViewportSize(int w, int h) override;

	Vec2 getMouseScreenPos() { return mousePos_; }
	Vec2 getMouseUIPos() { return mouseUIPos_; }
	bool hasMouseMoved() { return hasMouseMoved_; }

	void playSound(
		SoundAsset *asset, float volume,
		std::optional<Vec2> center) override;
	void playSound(
		SoundAsset *asset, float volume,
		std::optional<Vec2> center,
		SoundHandle &handle) override;

	Vec2 getMousePos();
	TilePos getMouseTile();

	void drawDebugMenu();
	void drawPerfMenu();
	void draw() override;
	void render() override;
	void screenshot(const char *path, int w = -1, int h = -1) override;

	void update(float dt) override;
	void onQuit() override;

	void save();

	InputHandler &inputs() override { return inputHandler_; }

	CommandSpec *matchCommand(std::span<CowStr> tokens, std::vector<CowStr> &out);
	void runCommand(Ctx &ctx, std::string_view command, std::string &out);

	std::unique_ptr<World> world_ = NULL;
	std::string worldPath_;
	Cygnet::RenderCamera cam_{.zoom = 1.0 / 8};
	Cygnet::RenderCamera uiCam_{.zoom = 1.0 / 16};

	std::string popupMessage_;
	float popupMessageTimer_ = 0;

	bool triggerSave_ = false;
	int triggerReload_ = 0;
	float timeScale_ = 1.0;
	std::optional<float> fixedDeltaTime_;
	float fpsLimit_ = 0;
	Perf perf_;
	std::vector<EntityRef> debugEntities_;
	std::unique_ptr<GameServer> server_;

	bool paused_ = false;
	bool shouldQuit_ = false;

	bool tickInProgress_ = false;
	RTDeadline tickDeadline_{2.0 / 1000};
	double fpsUpdateTime_ = std::chrono::duration<double>(
		std::chrono::steady_clock::now().time_since_epoch()).count();
	int frameCount_ = 0;
	int tickCount_ = 0;

	Action pauseAction_;
	Action entityDebugMenuAction_;
	Action debugMenuAction_;
	Action perfMenuAction_;
	Action reloadModsAction_;
	Action regenWorldAction_;
	Action uiActivateAction_;
	Action uiModAction_;
	Action uiCameraZoomAction_;

	std::vector<CommandSet> commandSets_;

private:
	bool reload();
	void tick();
	void initInputHandler();
	void initCommandHandler();

	float tickAcc_ = 0;

	Vec2 mousePos_;
	Vec2 mouseUIPos_;
	bool hasMouseMoved_ = false;

	std::vector<Item *> sortedItems_;
	bool hasSortedItems_ = false;

	SoundPlayer soundPlayer_;
	std::optional<FrameRecorder> frameRecorder_;
	InputHandler inputHandler_;

	double didScroll_ = 0;
	std::function<bool()> recompileMods_;
	HashMap<ModInfo> mods_;
	std::vector<std::string> modPaths_;

	std::vector<CowStr> commandTokensBuf_;
	std::vector<CowStr> commandArgvBuf_;
};

}
