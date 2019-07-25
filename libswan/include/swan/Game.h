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
	void loadMod(const std::string &path);
	void createWorld(std::string worldgen);

	void draw(Win &win);
	void update(float dt);
	void tick();

	static void initGlobal();

	std::unique_ptr<World> world_ = NULL;

private:
	std::vector<Mod> registered_mods_;
};

}
