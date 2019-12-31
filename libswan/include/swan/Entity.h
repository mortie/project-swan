#pragma once

#include <memory>
#include <optional>

#include "common.h"
#include "log.h"
#include "SRF.h"
#include "BoundingBox.h"
#include "Body.h"

namespace Swan {

class World;
class WorldPlane;
class Game;

class Entity {
public:
	class Factory {
	public:
		virtual ~Factory() = default;
		virtual Entity *create(const Context &ctx, const SRF &params) = 0;
		std::string name_;
	};

	virtual ~Entity() = default;

	virtual std::optional<BoundingBox> getBounds() { return std::nullopt; }

	virtual void draw(const Context &ctx, Win &win) {}
	virtual void update(const Context &ctx, float dt) {}
	virtual void tick(const Context &ctx, float dt) {}
	virtual void move(const Vec2 &pos) {}
	virtual void moveTo(const Vec2 &pos) {}
	virtual void despawn() {}
	virtual void readSRF(const Swan::Context &ctx, const SRF &srf) {}
	virtual SRF *writeSRF(const Swan::Context &ctx) { return new SRFNone(); }
};

class PhysicsEntity: public Entity {
public:
	PhysicsEntity(Vec2 size, float mass):
		body_(size, mass) {}

	virtual std::optional<BoundingBox> getBounds() override { return body_.getBounds(); }

	virtual void update(const Context &ctx, float dt) override {
		body_.friction();
		body_.gravity();
		body_.update(ctx.plane, dt);
	}

	virtual void move(const Vec2 &rel) override { body_.pos_ += rel; }
	virtual void moveTo(const Vec2 &pos) override { body_.pos_ = pos; }

protected:
	Body body_;
};

}
