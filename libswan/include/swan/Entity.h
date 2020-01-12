#pragma once

#include <memory>
#include <optional>

#include "common.h"
#include "log.h"
#include "traits/BodyTrait.h"
#include "SRF.h"

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

	virtual void draw(const Context &ctx, Win &win) {}
	virtual void update(const Context &ctx, float dt) {}
	virtual void tick(const Context &ctx, float dt) {}
	virtual void despawn() {}

	virtual void readSRF(const Swan::Context &ctx, const SRF &srf) {}
	virtual SRF *writeSRF(const Swan::Context &ctx) { return new SRFNone(); }
};

class PhysicsEntity: public Entity, public BodyTrait::HasBody {
public:
	PhysicsEntity(Vec2 size, float mass):
		body_(size, mass) {}

	virtual BodyTrait::Body &getBody() override { return body_; }

	virtual void update(const Context &ctx, float dt) override {
		body_.standardForces();
		body_.update(ctx, dt);
	}

protected:
	BodyTrait::PhysicsBody body_;
};

}
