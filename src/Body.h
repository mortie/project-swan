#pragma once

#include <SFML/Graphics.hpp>

#include "common.h"

class Body {
public:
	Vec2 force_ = { 0, 0 };
	Vec2 vel_ = { 0, 0 };
	Vec2 pos_;
	Vec2 size_;
	float mass_;

	Body(Vec2 pos, Vec2 size, float mass):
		pos_(pos), size_(size), mass_(mass) {};

	void friction(float coef);
	void gravity(Vec2 g = Vec2(0, 9.81));

	void outline(Win &win);
	void update(float dt);
};
