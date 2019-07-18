#pragma once

#include "common.h"
#include "Body.h"

namespace Swan {

class Player {
public:
	Player(Vec2 pos):
		body_(pos, size, mass) {}

	void draw(Win &win);
	void update(float dt);

private:
	static const float force;
	static const float friction;
	static const float mass;
	static const Vec2 size;

	Body body_;
};

}
