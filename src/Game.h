#pragma once

#include <vector>

#include "common.h"
#include "WorldPlane.h"
#include "Player.h"

class Game {
public:
	Player *player_;
	WorldPlane *current_plane_;
	std::vector<WorldPlane *> planes_;

	void draw(Win &win);
	void update(float dt);
	void tick();
};
