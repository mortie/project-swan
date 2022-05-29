#pragma once

#include <bitset>
#include <map>
#include <string>
#include <optional>
#include <GLFW/glfw3.h>
#include <cygnet/Renderer.h>
#include <cygnet/util.h>

#include "common.h"
#include "Mod.h"
#include "World.h"

namespace Swan {

class Game {
public:
	void createWorld(const std::string &worldgen, const std::vector<std::string> &modPaths);

	void onKeyDown(int scancode) {
		pressedKeys_[scancode] = true;
		didPressKeys_[scancode] = true;
	}

	void onKeyUp(int scancode) {
		pressedKeys_[scancode] = false;
		didReleaseKeys_[scancode] = true;
	}

	void onMouseMove(float x, float y) {
		mousePos_ = (Vec2{x, y} / (Vec2)cam_.size) * renderer_.winScale();
	}

	void onMouseDown(int button) {
		pressedButtons_[button] = true;
		didPressButtons_[button] = true;
	}

	void onMouseUp(int button) {
		pressedButtons_[button] = false;
		didReleaseButtons_[button] = true;
	}

	void onScrollWheel(double dy) {
		didScroll_ += dy;
	}

	bool isKeyPressed(int key) { return pressedKeys_[glfwGetKeyScancode(key)]; }
	bool wasKeyPressed(int key) { return didPressKeys_[glfwGetKeyScancode(key)]; }
	bool wasKeyReleased(int key) { return didReleaseKeys_[glfwGetKeyScancode(key)]; }
	Vec2 getMousePos() { return mousePos_; }
	bool isMousePressed(int button) { return pressedButtons_[button]; }
	bool wasMousePressed(int button) { return didPressButtons_[button]; }
	bool wasMouseReleased(int button) { return didReleaseButtons_[button]; }
	double wasWheelScrolled() { return didScroll_; }

	TilePos getMouseTile();

	Cygnet::Color backgroundColor();
	void draw();
	void update(float dt);
	void tick(float dt);

	std::unique_ptr<World> world_ = NULL;
	Cygnet::Renderer renderer_;
	Cygnet::RenderCamera cam_{.zoom = 0.125};

private:
	std::bitset<512> pressedKeys_;
	std::bitset<512> didPressKeys_;
	std::bitset<512> didReleaseKeys_;

	Vec2 mousePos_;
	std::bitset<8> pressedButtons_;
	std::bitset<8> didPressButtons_;
	std::bitset<8> didReleaseButtons_;

	double didScroll_ = 0;
};

}
