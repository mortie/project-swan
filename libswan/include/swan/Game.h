#pragma once

#include <bitset>
#include <chrono>
#include <string>
#include <span>
#include <cygnet/Renderer.h>
#include <cygnet/TextCache.h>
#include <cygnet/util.h>

#include <kj/io.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "common.h"
#include "World.h"
#include "SoundPlayer.h"
#include "FrameRecorder.h"

namespace Swan {

class Game {
public:
	using SoundHandle = std::shared_ptr<SoundPlayer::Handle>;

	void createWorld(
		const std::string &worldgen, std::span<std::string> modPaths);

	void loadWorld(
		kj::BufferedInputStream &is, std::span<std::string> modPaths);

	void onKeyDown(int scancode, int key);
	void onKeyUp(int scancode, int key);
	void onMouseMove(float x, float y);
	void onMouseDown(int button);
	void onMouseUp(int button);
	void onScrollWheel(double dy);

	bool isKeyPressed(int key)
	{
		int code = glfwGetKeyScancode(key);
		return pressedKeys_[code] || didPressKeys_[code];
	}

	bool wasKeyPressed(int key)
	{
		return didPressKeys_[glfwGetKeyScancode(key)];
	}

	bool wasKeyReleased(int key)
	{
		return didReleaseKeys_[glfwGetKeyScancode(key)];
	}

	bool isLiteralKeyPressed(int key)
	{
		return pressedLiteralKeys_[key];
	}

	bool wasLiteralKeyPressed(int key)
	{
		return didPressLiteralKeys_[key];
	}

	bool wasLiteralKeyReleased(int key)
	{
		return didReleaseLiteralKeys_[key];
	}

	Vec2 getMouseScreenPos()
	{
		return mousePos_;
	}

	Vec2 getMouseUIPos()
	{
		return mouseUIPos_;
	}

	bool isMousePressed(int button)
	{
		return pressedButtons_[button] || didPressButtons_[button];
	}

	bool wasMousePressed(int button)
	{
		return didPressButtons_[button];
	}

	bool wasMouseReleased(int button)
	{
		return didReleaseButtons_[button];
	}

	double wasWheelScrolled()
	{
		return didScroll_;
	}

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

	Cygnet::Color backgroundColor();

	void draw();
	void render();

	void update(float dt);
	void tick(float dt);
	void save();

	std::unique_ptr<World> world_ = NULL;
	Cygnet::Renderer renderer_;
	Cygnet::RenderCamera cam_{.zoom = 1.0 / 8};
	Cygnet::RenderCamera uiCam_{.zoom = 1.0 / 16};

	bool debugShowMenu_ = false;
	bool debugDrawCollisionBoxes_ = false;
	bool debugDrawChunkBoundaries_ = false;
	bool debugFluidParticleLocations_ = false;
	bool debugDisableShadows_ = false;
	bool debugHandBreakAny_ = false;
	bool debugOutputEntityProto_ = false;
	bool debugGodMode_ = false;
	bool triggerSave_ = false;
	bool enableVSync_ = false;
	float timeScale_ = 1.0;
	std::optional<float> fixedDeltaTime_;
	float fpsLimit_ = 0;

	int fps_ = 0;
	int frameAcc_ = 0;
	std::chrono::steady_clock::duration frameTimeAcc_;
	std::chrono::steady_clock::time_point fpsUpdateTime_ = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point prevTime_;

	std::shared_ptr<Cygnet::FontFace> notoSans_{Cygnet::loadFontFace(
		"assets/NotoSans-Regular.ttf")};

	Cygnet::TextCache smallFont_{notoSans_, 60};
	Cygnet::TextCache bigFont_{notoSans_, 200};

private:
	std::bitset<GLFW_KEY_LAST + 1> pressedKeys_;
	std::bitset<GLFW_KEY_LAST + 1> didPressKeys_;
	std::bitset<GLFW_KEY_LAST + 1> didReleaseKeys_;

	std::bitset<GLFW_KEY_LAST + 1> pressedLiteralKeys_;
	std::bitset<GLFW_KEY_LAST + 1> didPressLiteralKeys_;
	std::bitset<GLFW_KEY_LAST + 1> didReleaseLiteralKeys_;

	Vec2 mousePos_;
	Vec2 mouseUIPos_;
	std::bitset<GLFW_MOUSE_BUTTON_LAST + 1> pressedButtons_;
	std::bitset<GLFW_MOUSE_BUTTON_LAST + 1> didPressButtons_;
	std::bitset<GLFW_MOUSE_BUTTON_LAST + 1> didReleaseButtons_;

	std::vector<Item *> sortedItems_;
	bool hasSortedItems_ = false;

	SoundPlayer soundPlayer_;
	std::optional<FrameRecorder> frameRecorder_;

	double didScroll_ = 0;
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
