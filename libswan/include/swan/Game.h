#pragma once

#include <vector>
#include <map>
#include <string>

#include "common.h"
#include "World.h"
#include "Mod.h"

namespace Swan {

class Game {
public:
	Game(Win &win):
		mouse_pos_(0, 0),
		keys_pressed_(sf::Keyboard::Key::KeyCount, false),
		mouse_pressed_(sf::Mouse::Button::ButtonCount, false),
		win_(win) {}

	void loadMod(const std::string &path);
	void createWorld(std::string worldgen);

	void onKeyPressed(sf::Keyboard::Key key) { keys_pressed_[(int)key] = true; }
	void onKeyReleased(sf::Keyboard::Key key) { keys_pressed_[(int)key] = false; }
	void onMouseMove(int x, int y) { mouse_pos_ = Vec2i(x, y); }
	void onMousePressed(sf::Mouse::Button button) { mouse_pressed_[(int)button] = true; }
	void onMouseReleased(sf::Mouse::Button button) { mouse_pressed_[(int)button] = false; }

	TilePos getMouseTile();
	Vec2i getMousePos() { return mouse_pos_; }
	bool isKeyPressed(sf::Keyboard::Key key) { return keys_pressed_[(int)key]; }
	bool isMousePressed(sf::Mouse::Button button) { return mouse_pressed_[(int)button]; }

	void draw();
	void update(float dt);
	void tick();

	static void initGlobal();

	std::unique_ptr<World> world_ = NULL;

private:
	Vec2i mouse_pos_;
	std::vector<bool> keys_pressed_;
	std::vector<bool> mouse_pressed_;
	std::vector<Mod> registered_mods_;
	Win &win_;
};

}
