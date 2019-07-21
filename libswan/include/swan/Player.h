#pragma once

#include "common.h"
#include "Body.h"
#include "WorldPlane.h"

namespace Swan {

class Player {
public:
	Player(Vec2 pos):
		body_(pos, SIZE, MASS) {}

	void draw(Win &win);
	void update(WorldPlane &plane, float dt);

private:
	static const float FORCE;
	static const float FRICTION;
	static const float MASS;
	static const Vec2 SIZE;

	Body body_;
};

}
