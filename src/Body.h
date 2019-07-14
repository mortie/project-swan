#pragma once

#include <SFML/Graphics.hpp>

#include "common.h"

class Body {
public:
	Vec2 force_ = { 0, 0 };
	Vec2 vel_ = { 0, 0 };
	Vec2 pos_;
	Vec2 size_;

	Body(Vec2 pos, Vec2 size):
		pos_(pos), size_(size) {};

	void outline(Win &win);
	void update(float dt);
};
