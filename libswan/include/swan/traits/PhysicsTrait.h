#pragma once

#include "../traits/BodyTrait.h"
#include "../common.h"

namespace Swan {

struct PhysicsTrait {
	struct Tag {};

	struct Physics {
		virtual void applyForce(Vec2 force) = 0;
		virtual void addVelocity(Vec2 vel) = 0;
		virtual Vec2 getVelocity() = 0;

	protected:
		~Physics() = default;
	};

	virtual Physics &get(Tag) = 0;

protected:
	~PhysicsTrait() = default;
};

struct BasicPhysics final: public PhysicsTrait::Physics {
	struct Props {
		float mass;
		float bounciness = 0;
		float mushyness = 2;
	};

	Vec2 vel{};
	Vec2 force{};
	bool onGround = false;

	void friction(Vec2 coef = Vec2(400, 50));
	void gravity(float mass, Vec2 g = Vec2(0, 20));
	void standardForces(float mass) { friction(); gravity(mass); }

	void applyForce(Vec2 f) override;
	void addVelocity(Vec2 v) override;
	Vec2 getVelocity() override { return vel; }

	void update(
		const Swan::Context &ctx, float dt,
		BodyTrait::Body &body, const Props &props);
};

/*
 * BasicPhysics
 */

inline void BasicPhysics::friction(Vec2 coef) {
	force += -vel * coef;
}

inline void BasicPhysics::gravity(float mass, Vec2 g) {
	force += g * mass;
}

inline void BasicPhysics::applyForce(Vec2 f) {
	force += f;
}

inline void BasicPhysics::addVelocity(Vec2 v) {
	vel += v;
}

}
