#pragma once

#include <bitset>
#include <istream>
#include <string>
#include <cygnet/Renderer.h>
#include <cygnet/util.h>
#include <msgstream/msgstream.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "common.h"
#include "World.h"
#include "SoundPlayer.h"

namespace Swan {

class Game {
public:
	void createWorld(
		const std::string &worldgen, const std::vector<std::string> &modPaths);

	void loadWorld(
		std::istream &is, const std::vector<std::string> &modPaths);

	void onKeyDown(int scancode, int key)
	{
		pressedKeys_[scancode] = true;
		didPressKeys_[scancode] = true;
		if (key >= 0) {
			pressedLiteralKeys_[key] = true;
			didPressLiteralKeys_[key] = true;
		}
	}

	void onKeyUp(int scancode, int key)
	{
		pressedKeys_[scancode] = false;
		if (key >= 0) {
			pressedLiteralKeys_[key] = false;
		}
	}

	void onMouseMove(float x, float y)
	{
		mousePos_ = (Vec2{x, y} / (Vec2)cam_.size) * renderer_.winScale();
	}

	void onMouseDown(int button)
	{
		pressedButtons_[button] = true;
		didPressButtons_[button] = true;
	}

	void onMouseUp(int button)
	{
		pressedButtons_[button] = false;
		didReleaseButtons_[button] = true;
	}

	void onScrollWheel(double dy)
	{
		didScroll_ += dy;
	}

	bool isKeyPressed(int key)
	{
		return pressedKeys_[glfwGetKeyScancode(key)];
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

	bool isMousePressed(int button)
	{
		return pressedButtons_[button];
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

	void playSound(SoundAsset *asset)
	{
		soundPlayer_.play(asset);
	}

	void playSound(SoundAsset *asset, std::shared_ptr<SoundPlayer::Handle> h)
	{
		soundPlayer_.play(asset, h);
	}

	Vec2 getMousePos();
	TilePos getMouseTile();

	Cygnet::Color backgroundColor();
	void draw();

	void render()
	{
		renderer_.render(cam_);
		renderer_.renderUI(uiCam_);
	}

	void update(float dt);
	void tick(float dt);
	void save();

	std::unique_ptr<World> world_ = NULL;
	Cygnet::Renderer renderer_;
	Cygnet::RenderCamera cam_{.zoom = 1.0/8};
	Cygnet::RenderCamera uiCam_{.zoom = 1.0/16};

	bool debugShowMenu_ = false;
	bool debugDrawCollisionBoxes_ = false;
	bool debugDrawChunkBoundaries_ = false;
	bool enableVSync_ = true;
	float timeScale_ = 1.0;

private:
	std::bitset<GLFW_KEY_LAST + 1> pressedKeys_;
	std::bitset<GLFW_KEY_LAST + 1> didPressKeys_;
	std::bitset<GLFW_KEY_LAST + 1> didReleaseKeys_;

	std::bitset<GLFW_KEY_LAST + 1> pressedLiteralKeys_;
	std::bitset<GLFW_KEY_LAST + 1> didPressLiteralKeys_;
	std::bitset<GLFW_KEY_LAST + 1> didReleaseLiteralKeys_;

	Vec2 mousePos_;
	std::bitset<GLFW_MOUSE_BUTTON_LAST + 1> pressedButtons_;
	std::bitset<GLFW_MOUSE_BUTTON_LAST + 1> didPressButtons_;
	std::bitset<GLFW_MOUSE_BUTTON_LAST + 1> didReleaseButtons_;

	SoundPlayer soundPlayer_;

	double didScroll_ = 0;
};

}
