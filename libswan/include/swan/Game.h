#pragma once

#include <vector>
#include <map>
#include <string>

#include "common.h"
#include "World.h"

namespace Swan {

class Game {
public:
	Game(Win &win):
		win_(win),
		mouse_pos_(0, 0) {}

	void createWorld(const std::string &worldgen);

	void onKeyDown(SDL_Keysym sym) { pressed_keys_[sym.scancode] = true; }
	void onKeyUp(SDL_Keysym sym) { pressed_keys_[sym.scancode] = false; }
	void onMouseMove(Sint32 x, Sint32 y) { mouse_pos_ = { x, y }; }
	void onMouseDown(Sint32 x, Sint32 y, Uint8 button) { mouse_pos_ = { x, y }; pressed_buttons_[button] = true; }
	void onMouseUp(Sint32 x, Sint32 y, Uint8 button) { mouse_pos_ = { x, y }; pressed_buttons_[button] = false; }

	bool isKeyPressed(SDL_Scancode code) { return pressed_keys_[code]; }
	TilePos getMouseTile();
	Vec2i getMousePos() { return mouse_pos_; }
	bool isMousePressed(Uint8 button) { return pressed_buttons_[button]; }

	void draw();
	void update(float dt);
	void tick(float dt);

	std::unique_ptr<World> world_ = NULL;
	std::unique_ptr<ImageResource> invalid_image_ = NULL;
	std::unique_ptr<Tile> invalid_tile_ = NULL;
	std::unique_ptr<Item> invalid_item_ = NULL;
	Win &win_;

private:
	std::unordered_map<SDL_Scancode, bool> pressed_keys_;
	Vec2i mouse_pos_;
	std::unordered_map<Uint8, bool> pressed_buttons_;
};

}
