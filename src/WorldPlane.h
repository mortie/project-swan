#pragma once

#include "common.h"

class WorldPlane {
public:
	void draw(Win &win);
	void update(float dt);
	void tick();
};
