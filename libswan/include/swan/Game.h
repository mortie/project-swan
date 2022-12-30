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

	void onKeyDown(int scancode, int key) {
		pressedKeys_[scancode] = true;
		didPressKeys_[scancode] = true;
		pressedLiteralKeys_[key] = true;
		didPressLiteralKeys_[key] = true;
	}

	void onKeyUp(int scancode, int key) {
		pressedKeys_[scancode] = false;
		pressedLiteralKeys_[key] = false;
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
	bool isLiteralKeyPressed(int key) { return pressedLiteralKeys_[key]; }
	bool wasLiteralKeyPressed(int key) { return didPressLiteralKeys_[key]; }
	bool wasLiteralKeyReleased(int key) { return didReleaseLiteralKeys_[key]; }
	Vec2 getMousePos() { return mousePos_; }
	bool isMousePressed(int button) { return pressedButtons_[button]; }
	bool wasMousePressed(int button) { return didPressButtons_[button]; }
	bool wasMouseReleased(int button) { return didReleaseButtons_[button]; }
	double wasWheelScrolled() { return didScroll_; }

	TilePos getMouseTile();

	Cygnet::Color backgroundColor();
	void draw();
	void render() { renderer_.draw(cam_); }
	void update(float dt);
	void tick(float dt);

	std::unique_ptr<World> world_ = NULL;
	Cygnet::Renderer renderer_;
	Cygnet::RenderCamera cam_{.zoom = 0.125};

	bool debugShowMenu_ = false;
	bool debugDrawCollisionBoxes_ = false;

private:
	std::bitset<GLFW_KEY_LAST> pressedKeys_;
	std::bitset<GLFW_KEY_LAST> didPressKeys_;
	std::bitset<GLFW_KEY_LAST> didReleaseKeys_;

	std::bitset<GLFW_KEY_LAST> pressedLiteralKeys_;
	std::bitset<GLFW_KEY_LAST> didPressLiteralKeys_;
	std::bitset<GLFW_KEY_LAST> didReleaseLiteralKeys_;

	Vec2 mousePos_;
	std::bitset<GLFW_MOUSE_BUTTON_LAST> pressedButtons_;
	std::bitset<GLFW_MOUSE_BUTTON_LAST> didPressButtons_;
	std::bitset<GLFW_MOUSE_BUTTON_LAST> didReleaseButtons_;

	double didScroll_ = 0;
};

}
