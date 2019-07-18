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
	std::vector<Mod> registered_mods_;

	World *world_;

	void loadMod(const std::string &path);

	void draw(Win &win);
	void update(float dt);
	void tick();
};

}
