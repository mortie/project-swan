#pragma once

#include "Entity.h"
#include "traits/BodyTrait.h"
#include "traits/PhysicsTrait.h"

namespace Swan {

class PhysicsEntity: public Entity, public BodyTrait, public PhysicsTrait {
public:
	PhysicsEntity(Vec2 size): body_({.size = size}) {}

	BodyTrait::Body &get(BodyTrait::Tag) override { return body_; }
	PhysicsTrait::Physics &get(PhysicsTrait::Tag) override { return physics_; }

	void physics(
			const Context &ctx, float dt,
			const BasicPhysics::Props &props) {

		physics_.standardForces(props.mass);
		physics_.update(ctx, dt, body_, props);
	}

protected:
	BodyTrait::Body body_;
	BasicPhysics physics_;
};

}
