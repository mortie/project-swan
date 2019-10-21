#pragma once

#include <SFML/Graphics.hpp>

#include "common.h"
#include "BoundingBox.h"

namespace Swan {

class WorldPlane;

class Body {
public:
	Body(Vec2 size, float mass, Vec2 pos = Vec2::ZERO):
		size_(size), mass_(mass), pos_(pos) {};

	void friction(Vec2 coef = Vec2(400, 50));
	void gravity(Vec2 g = Vec2(0, 20));
	void collide(WorldPlane &plane);

	void outline(Win &win);
	void update(float dt);

	BoundingBox getBounds() { return { pos_, size_ }; }

	Vec2 force_ = { 0, 0 };
	Vec2 vel_ = { 0, 0 };
	Vec2 size_;
	float mass_;
	Vec2 pos_;
	bool on_ground_ = false;
};

}
