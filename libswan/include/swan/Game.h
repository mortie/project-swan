#pragma once

#include <bitset>
#include <map>
#include <string>
#include <SDL.h>

#include "common.h"
#include "Resource.h"
#include "Mod.h"
#include "World.h"

namespace Swan {

class Game {
public:
	Game(Win &win):
		win_(win),
		mouse_pos_(0, 0) {}

	std::unique_ptr<Mod> loadMod(const std::string &path, World &world);
	void createWorld(const std::string &worldgen, std::vector<std::string> mods);

	void onKeyDown(SDL_Keysym sym) {
		pressed_keys_[sym.scancode] = true;
		did_press_keys_[sym.scancode] = true;
	}

	void onKeyUp(SDL_Keysym sym) {
		pressed_keys_[sym.scancode] = false;
		did_release_keys_[sym.scancode] = true;
	}

	void onMouseMove(Sint32 x, Sint32 y) {
		mouse_pos_ = { x, y };
	}

	void onMouseDown(Sint32 x, Sint32 y, Uint8 button) {
		mouse_pos_ = { x, y };
		pressed_buttons_[button] = true;
		did_press_buttons_[button] = true;

}
	void onMouseUp(Sint32 x, Sint32 y, Uint8 button) {
		mouse_pos_ = { x, y };
		pressed_buttons_[button] = false;
		did_release_buttons_[button] = true;
	}

	void onScrollWheel(Sint32 y) {
		did_scroll_ = (y > 0 ? 1 : -1 );
	}

	bool isKeyPressed(SDL_Scancode code) { return pressed_keys_[code]; }
	bool wasKeyPressed(SDL_Scancode code) { return did_press_keys_[code]; }
	bool wasKeyReleased(SDL_Scancode code) { return did_release_keys_[code]; }
	Vec2i getMousePos() { return mouse_pos_; }
	bool isMousePressed(Uint8 button) { return pressed_buttons_[button]; }
	bool wasMousePressed(Uint8 button) { return did_press_buttons_[button]; }
	bool wasMouseReleased(Uint8 button) { return did_release_buttons_[button]; }
	int wasWheelScrolled() { return did_scroll_; }

	TilePos getMouseTile();

	SDL_Color backgroundColor();
	void draw();
	void update(float dt);
	void tick(float dt);

	std::unique_ptr<World> world_ = NULL;
	std::unique_ptr<ImageResource> invalid_image_ = NULL;
	std::unique_ptr<Tile> invalid_tile_ = NULL;
	std::unique_ptr<Item> invalid_item_ = NULL;
	Win &win_;

private:
	std::bitset<SDL_NUM_SCANCODES> pressed_keys_;
	std::bitset<SDL_NUM_SCANCODES> did_press_keys_;
	std::bitset<SDL_NUM_SCANCODES> did_release_keys_;

	Vec2i mouse_pos_;
	std::bitset<SDL_BUTTON_X2> pressed_buttons_;
	std::bitset<SDL_BUTTON_X2> did_press_buttons_;
	std::bitset<SDL_BUTTON_X2> did_release_buttons_;

	int did_scroll_ = 0;
};

}
