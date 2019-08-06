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
	Game(Win &win): win_(win) {}

	void loadMod(const std::string &path);
	void createWorld(std::string worldgen);

	TilePos getMouseTile();

	void draw();
	void update(float dt);
	void tick();

	static void initGlobal();

	std::unique_ptr<World> world_ = NULL;

private:
	std::vector<Mod> registered_mods_;
	Win &win_;
};

}
