#pragma once

#include "../traits/BodyTrait.h"
#include "../common.h"

namespace Swan {

struct PhysicsTrait {
	struct Physics;
	struct Tag {};
	virtual Physics &get(Tag) = 0;

	struct PhysicsProps {
		float mass;
		float bounciness = 0;
		float mushyness = 2;
	};

	struct Physics {
		Vec2 vel{};
		Vec2 force{};
		bool onGround = false;

		void friction(Vec2 coef = Vec2(400, 50));
		void gravity(float mass, Vec2 g = Vec2(0, 20));
		void standardForces(float mass) { friction(); gravity(mass); }

		void update(
			const Swan::Context &ctx, float dt,
			BodyTrait::Body &body, const PhysicsProps &props);
	};
};

/*
 * Physics
 */

inline void PhysicsTrait::Physics::friction(Vec2 coef) {
	force += -vel * coef;
}

inline void PhysicsTrait::Physics::gravity(float mass, Vec2 g) {
	force += g * mass;
}

}
