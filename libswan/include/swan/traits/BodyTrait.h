#pragma once

#include "../common.h"
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

	float left() { return pos.x; }
	float right() { return pos.x + size.x; }
	float midX() { return pos.x + size.x / 2; }
	float top() { return pos.y; }
	float bottom() { return pos.y + size.y; }
	float midY() { return pos.y + size.y / 2; }

	Vec2 topLeft() { return { left(), top() }; }
	Vec2 midLeft() { return { left(), midY() }; }
	Vec2 bottomLeft() { return { left(), bottom() }; }
	Vec2 topMid() { return { midX(), top() }; }
	Vec2 center() { return { midX(), midY() }; }
	Vec2 bottomMid() { return { midX(), bottom() }; }
	Vec2 topRight() { return { right(), top() }; }
	Vec2 midRight() { return { right(), midY() }; }
	Vec2 bottomRight() { return { right(), bottom() }; }
};

class Body {
public:
	virtual ~Body() = default;

	virtual Bounds getBounds() = 0;
	virtual void move(Vec2 rel) = 0;
	virtual void moveTo(Vec2 pos) = 0;
};

// PhysicsBody is a BodyTrait::Body which implements
// a bunch of physics stuff.
class PhysicsBody: public Body {
public:
	PhysicsBody(Vec2 size, float mass, Vec2 pos = Vec2::ZERO):
		size_(size), mass_(mass), pos_(pos) {};

	BodyTrait::Bounds getBounds() override { return BodyTrait::Bounds{ pos_, size_ }; }
	void move(Vec2 rel) override { pos_ += rel; }
	void moveTo(Vec2 pos) override { pos_ = pos; }

	void friction(Vec2 coef = Vec2(400, 50));
	void gravity(Vec2 g = Vec2(0, 20));
	void standardForces() { friction(); gravity(); }

	void outline(Win &win);
	void update(const Swan::Context &ctx, float dt);
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

// StaticBody is a BodyTrait::Body which just has a static
// position and size.
class StaticBody: public Body {
public:
	StaticBody(Vec2 size, Vec2 pos):
		size_(size), pos_(pos) {}

	BodyTrait::Bounds getBounds() override { return BodyTrait::Bounds{ pos_, size_ }; }
	void move(Vec2 rel) override { pos_ += rel; }
	void moveTo(Vec2 pos) override { pos_ = pos; }

	Vec2 size_;
	Vec2 pos_;
};

}
}
