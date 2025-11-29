#pragma once

#include "Clock.h"
#include <bitset>
#include <chrono>
#include <string>
#include <span>
#include <cygnet/Renderer.h>
#include <cygnet/TextCache.h>
#include <cygnet/util.h>

#include <kj/io.h>

#include "InputHandler.h"
#include "common.h"
#include "World.h"
#include "SoundPlayer.h"
#include "FrameRecorder.h"

namespace Swan {

class Game {
public:
	Game(std::function<bool()> recompileMods): recompileMods_(recompileMods) {}

	struct Debug {
		bool show = false;
		bool drawCollisionBoxes = false;
		bool drawChunkBoundaries = false;
		bool drawWorldTicks = false;
		bool fluidParticleLocations = false;
		bool disableShadows = false;
		bool handBreakAny = false;
		bool outputEntityProto = false;
		bool godMode = false;
		bool infiniteItems = false;
		bool showInputDebug = false;
	};

	struct PerfRecord {
		float maxMs = 0;
		float avgMs = 0;
		float nextMaxSec = 0;
		float nextSumSec = 0;

		void record(float sec)
		{
			nextSumSec += sec;
			if (sec > nextMaxSec) {
				nextMaxSec = sec;
			}
		}

		void capture(int num)
		{
			maxMs = nextMaxSec * 1000;
			nextMaxSec = 0;
			avgMs = (nextSumSec / num) * 1000;
			nextSumSec = 0;
		}
	};

	struct Perf {
		bool show = false;
		int updateCount = 0;
		int tickCount = 0;

		int fps = 0;
		int tps = 0;
		PerfRecord entityUpdateTime;
		PerfRecord entityTickTime;
		PerfRecord tileTickTime;
		PerfRecord fluidTickTime;
		PerfRecord fluidUpdateTime;
		PerfRecord worldTickTime;
	};

	void createWorld(
		std::string worldPath, const std::string &worldgen,
		uint32_t seed, std::span<std::string> modPaths);

	void loadWorld(
		std::string worldPath, std::span<const std::string> modPaths);

	void onMouseMove(float x, float y);
	void onScrollWheel(double dy);

	Vec2 getMouseScreenPos() { return mousePos_; }
	Vec2 getMouseUIPos() { return mouseUIPos_; }
	bool hasMouseMoved() { return hasMouseMoved_; }

	void playSound(SoundAsset *asset);
	void playSound(SoundAsset *asset, float volume);
	void playSound(SoundAsset *asset, Vec2 center);
	void playSound(SoundAsset *asset, float volume, Vec2 center);
	void playSound(SoundAsset *asset, SoundHandle handle);
	void playSound(SoundAsset *asset, float volume, SoundHandle handle);
	void playSound(SoundAsset *asset, Vec2 center, SoundHandle handle);
	void playSound(SoundAsset *asset, float volume, Vec2 center, SoundHandle handle);

	void spawnParticle(Cygnet::RenderLayer layer, Cygnet::Renderer::SpawnParticle p)
	{
		renderer_.spawnParticle(layer, p);
	}
	void spawnParticle(Cygnet::Renderer::SpawnParticle p)
	{
		renderer_.spawnParticle(p);
	}

	Vec2 getMousePos();
	TilePos getMouseTile();

	void drawDebugMenu();
	void drawPerfMenu();
	void draw();
	void render();
	void screenshot(const char *path, int w = -1, int h = -1);

	void update(float dt);
	void save();

	InputHandler &inputs() { return inputHandler_; }
	Action *action(std::string_view name) { return inputs().action(name); }

	std::unique_ptr<World> world_ = NULL;
	std::string worldPath_;
	Cygnet::Renderer renderer_;
	Cygnet::RenderCamera cam_{.zoom = 1.0 / 8};
	Cygnet::RenderCamera uiCam_{.zoom = 1.0 / 16};

	std::string popupMessage_;
	float popupMessageTimer_ = 0;

	bool triggerSave_ = false;
	int triggerReload_ = 0;
	bool enableVSync_ = false;
	float timeScale_ = 1.0;
	std::optional<float> fixedDeltaTime_;
	float fpsLimit_ = 0;
	Debug debug_;
	Perf perf_;
	std::vector<EntityRef> debugEntities_;

	bool tickInProgress_ = false;
	RTDeadline tickDeadline_{2.0 / 1000};
	double fpsUpdateTime_ = std::chrono::duration<double>(
		std::chrono::steady_clock::now().time_since_epoch()).count();
	int frameCount_ = 0;
	int tickCount_ = 0;

	std::shared_ptr<Cygnet::FontFace> notoSans_{Cygnet::loadFontFace(
		"assets/NotoSans-Regular.ttf")};

	Cygnet::TextCache smallFont_{notoSans_, 60};
	Cygnet::TextCache bigFont_{notoSans_, 200};

	Action *entityDebugMenuAction_;
	Action *debugMenuAction_;
	Action *perfMenuAction_;
	Action *reloadModsAction_;
	Action *regenWorldAction_;

private:
	bool reload();
	void tick();
	void initInputHandler();

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
};


inline void Game::playSound(SoundAsset *asset)
{
	soundPlayer_.play(asset, 0.5, {});
}

inline void Game::playSound(SoundAsset *asset, float volume)
{
	soundPlayer_.play(asset, volume, {});
}

inline void Game::playSound(SoundAsset *asset, Vec2 center)
{
	soundPlayer_.play(asset, 0.5, std::pair{center.x, center.y});
}

inline void Game::playSound(SoundAsset *asset, float volume, Vec2 center)
{
	soundPlayer_.play(asset, volume, std::pair{center.x, center.y});
}

inline void Game::playSound(SoundAsset *asset, SoundHandle handle)
{
	soundPlayer_.play(asset, 0.5, {}, std::move(handle));
}

inline void Game::playSound(SoundAsset *asset, float volume, SoundHandle handle)
{
	soundPlayer_.play(asset, volume, {}, std::move(handle));
}

inline void Game::playSound(SoundAsset *asset, Vec2 center, SoundHandle handle)
{
	soundPlayer_.play(
		asset, 0.5, std::pair{center.x, center.y}, std::move(handle));
}

inline void Game::playSound(
	SoundAsset *asset, float volume, Vec2 center, SoundHandle handle)
{
	soundPlayer_.play(
		asset, volume, std::pair{center.x, center.y}, std::move(handle));
}

}
