#pragma once

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

	void outline(Win &win);
	void update(WorldPlane &plane, float dt);
	void updateWithoutCollision(float dt);

	BoundingBox getBounds() { return { pos_, size_ }; }

	Vec2 force_ = { 0, 0 };
	Vec2 vel_ = { 0, 0 };
	bool on_ground_ = false;
	Vec2 size_;
	float mass_;
	Vec2 pos_;
	float bounciness_ = 0;
	float mushyness_ = 2;

private:
	void collideX(WorldPlane &plane);
	void collideY(WorldPlane &plane);
};

}
