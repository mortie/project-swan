#pragma once

#include <optional>
#include <cygnet/Gui.h>
#include <cygnet/Renderer.h>
#include <swan/Vector2.h>

#include "WorldPlane.h"

namespace Swan {

struct SoundAsset;
class SoundHandle;
class InputHandler;
class GameServer;

class GameIO {
public:
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

	virtual ~GameIO() = default;

	virtual InputHandler &inputs() = 0;
	virtual void onMouseMove(float x, float y) = 0;
	virtual void onScrollWheel(float dy) = 0;
	virtual void onViewportSize(int w, int h) = 0;

	virtual void update(float dt) = 0;
	virtual void draw() = 0;
	virtual void render() = 0;
	virtual void screenshot(const char *path, int w, int h) = 0;
	virtual void onQuit() = 0;

	Vec2 getMouseScreenPos() { return mousePos_; }
	Vec2 getMouseUIPos() { return mouseUIPos_; }
	bool hasMouseMoved() { return hasMouseMoved_; }
	virtual Vec2 getMousePos() = 0;
	virtual TilePos getMouseTile() = 0;
	virtual Vec2 uiPosFromWorldPos(Vec2 worldPos) = 0;

	virtual void onTileChange(
		WorldPlane::ID plane,
		TilePos pos,
		Tile::ID newID)
	{}
	virtual void onBackgroundTileChange(
		WorldPlane::ID plane,
		TilePos pos,
		Tile::ID newID)
	{}

	virtual void playSound(
		SoundAsset *asset, float volume,
		std::optional<Vec2> center) = 0;
	virtual void playSound(
		SoundAsset *asset, float volume,
		std::optional<Vec2> center,
		SoundHandle &handle) = 0;

	void playSound(SoundAsset *asset);
	void playSound(SoundAsset *asset, float volume);
	void playSound(SoundAsset *asset, Vec2 center);
	void playSound(SoundAsset *asset, SoundHandle &handle);
	void playSound(SoundAsset *asset, float volume, SoundHandle &handle);
	void playSound(SoundAsset *asset, Vec2 center, SoundHandle &handle);

	void spawnParticle(
		Cygnet::RenderLayer layer,
		Cygnet::Renderer::SpawnParticle p)
	{
		renderer_.spawnParticle(layer, p);
	}
	void spawnParticle(Cygnet::Renderer::SpawnParticle p)
	{
		renderer_.spawnParticle(p);
	}

	bool shouldQuit_ = false;
	float fpsLimit_ = 0;
	float fixedDeltaTime_ = 0;
	float timeScale_ = 1;
	bool vsync_ = false;
	Debug debug_;
	Perf perf_;
	Cygnet::Renderer renderer_;
	Cygnet::Gui gui_{&renderer_};

	std::shared_ptr<Cygnet::FontFace> notoSans_{Cygnet::loadFontFace(
		"assets/NotoSans-Regular.ttf")};

	Cygnet::TextCache smallFont_{notoSans_, 60};
	Cygnet::TextCache bigFont_{notoSans_, 200};

	Vec2 mousePos_;
	Vec2 mouseUIPos_;
	bool hasMouseMoved_ = false;
};

inline void GameIO::playSound(SoundAsset *asset)
{
	playSound(asset, 0.5, {});
}

inline void GameIO::playSound(SoundAsset *asset, float volume)
{
	playSound(asset, volume, {});
}

inline void GameIO::playSound(SoundAsset *asset, Vec2 center)
{
	playSound(asset, 0.5, center);
}

inline void GameIO::playSound(SoundAsset *asset, SoundHandle &handle)
{
	playSound(asset, 0.5, {}, handle);
}

inline void GameIO::playSound(SoundAsset *asset, Vec2 center, SoundHandle &handle)
{
	playSound(asset, 0.5, center, handle);
}

}
