#pragma once

#include <SFML/Graphics.hpp>

#include "common.h"
#include "Body.h"

class Player {
public:
	Player(Vec2 pos):
		body_(pos, Vec2(1, 1)) {}

	void draw(Win &win);
	void update(float dt);

private:
	Body body_;
};
