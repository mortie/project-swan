#pragma once

#include <bitset>
#include <map>
#include <string>
#include <optional>
#include <SDL.h>
#include <cygnet/Renderer.h>

#include "common.h"
#include "Mod.h"
#include "World.h"

namespace Swan {

class Game {
public:
	void createWorld(const std::string &worldgen, const std::vector<std::string> &modPaths);

	void onKeyDown(SDL_Keysym sym) {
		pressedKeys_[sym.scancode] = true;
		didPressKeys_[sym.scancode] = true;
	}

	void onKeyUp(SDL_Keysym sym) {
		pressedKeys_[sym.scancode] = false;
		didReleaseKeys_[sym.scancode] = true;
	}

	void onMouseMove(Sint32 x, Sint32 y) {
		mousePos_ = { x, y };
	}

	void onMouseDown(Sint32 x, Sint32 y, Uint8 button) {
		mousePos_ = { x, y };
		pressedButtons_[button] = true;
		didPressButtons_[button] = true;

}
	void onMouseUp(Sint32 x, Sint32 y, Uint8 button) {
		mousePos_ = { x, y };
		pressedButtons_[button] = false;
		didReleaseButtons_[button] = true;
	}

	void onScrollWheel(Sint32 y) {
		didScroll_ = (y > 0 ? 1 : -1 );
	}

	bool isKeyPressed(SDL_Scancode code) { return pressedKeys_[code]; }
	bool wasKeyPressed(SDL_Scancode code) { return didPressKeys_[code]; }
	bool wasKeyReleased(SDL_Scancode code) { return didReleaseKeys_[code]; }
	Vec2i getMousePos() { return mousePos_; }
	bool isMousePressed(Uint8 button) { return pressedButtons_[button]; }
	bool wasMousePressed(Uint8 button) { return didPressButtons_[button]; }
	bool wasMouseReleased(Uint8 button) { return didReleaseButtons_[button]; }
	int wasWheelScrolled() { return didScroll_; }

	TilePos getMouseTile();

	SDL_Color backgroundColor();
	void draw();
	void update(float dt);
	void tick(float dt);

	std::unique_ptr<World> world_ = NULL;
	Cygnet::Renderer renderer_;
	Cygnet::RenderCamera cam_;

private:
	std::bitset<SDL_NUM_SCANCODES> pressedKeys_;
	std::bitset<SDL_NUM_SCANCODES> didPressKeys_;
	std::bitset<SDL_NUM_SCANCODES> didReleaseKeys_;

	Vec2i mousePos_;
	std::bitset<SDL_BUTTON_X2> pressedButtons_;
	std::bitset<SDL_BUTTON_X2> didPressButtons_;
	std::bitset<SDL_BUTTON_X2> didReleaseButtons_;

	int didScroll_ = 0;
};

}
