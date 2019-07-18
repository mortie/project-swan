#pragma once

#include <SFML/Graphics.hpp>

#include "common.h"
#include "Body.h"

class Player {
public:
	Player(Vec2 pos):
		body_(pos, Vec2(width, height), mass) {}

	void draw(Win &win);
	void update(float dt);

private:
	static constexpr float force = 600;
	static constexpr float friction = 100;
	static constexpr float mass = 80;
	static constexpr float width = 1;
	static constexpr float height = 2;

	Body body_;
};
