#pragma once

#include "../Vector2.h"

namespace Swan {

class WorldPlane;
class Win;

namespace BodyTrait {

class Body;
class HasBody {
public:
	virtual Body &getBody() = 0;
};

struct Bounds {
	Vec2 pos;
	Vec2 size;

	double left() { return pos.x; }
	double right() { return pos.x + size.x; }
	double top() { return pos.y; }
	double bottom() { return pos.y + size.y; }
};

class Body {
public:
	virtual Bounds getBounds() = 0;
	virtual void move(Vec2 rel) = 0;
	virtual void moveTo(Vec2 pos) = 0;
};

// PhysicsBody is an implementation of BodyTrait::Body which implements
// a bunch of physics stuff.
class PhysicsBody: public Body {
public:
	PhysicsBody(Vec2 size, float mass, Vec2 pos = Vec2::ZERO):
		size_(size), mass_(mass), pos_(pos) {};

	BodyTrait::Bounds getBounds() override { return BodyTrait::Bounds{ pos_, size_ }; };
	void move(Vec2 rel) { pos_ += rel; }
	void moveTo(Vec2 pos) { pos_ = pos; }

	void friction(Vec2 coef = Vec2(400, 50));
	void gravity(Vec2 g = Vec2(0, 20));

	void outline(Win &win);
	void update(WorldPlane &plane, float dt);
	void updateWithoutCollision(float dt);

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
}
