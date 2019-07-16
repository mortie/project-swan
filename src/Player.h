#pragma once

#include <SFML/Graphics.hpp>

#include "common.h"
#include "Body.h"

class Player {
public:
	Player(Vec2 pos):
		body_(pos, Vec2(1, 1), mass) {}

	void draw(Win &win);
	void update(float dt);

private:
	static constexpr float force = 600;
	static constexpr float friction = 100;
	static constexpr float mass = 50;

	Body body_;
};
