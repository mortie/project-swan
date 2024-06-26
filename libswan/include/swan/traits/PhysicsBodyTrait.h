#pragma once

#include <sbon.h>

#include "../traits/BodyTrait.h"
#include "../common.h"

namespace Swan {

struct PhysicsBodyTrait: public BodyTrait {
	struct Tag {};

	struct PhysicsBody {
		virtual void applyForce(Vec2 force) = 0;
		virtual void addVelocity(Vec2 vel) = 0;
		virtual Vec2 velocity() = 0;

	protected:
		~PhysicsBody() = default;
	};

	using BodyTrait::get;
	virtual PhysicsBody &get(Tag) = 0;

protected:
	~PhysicsBodyTrait() = default;
};

struct BasicPhysicsBody final: public PhysicsBodyTrait::PhysicsBody {
	struct Props {
		Vec2 size;
		float mass;
		float bounciness = 0.6;
		float mushyness = 2;
		bool isSolid = true;
	};

	BasicPhysicsBody(Props props):
		body({.size = props.size, .isSolid = props.isSolid}),
		mass(props.mass),
		bounciness(props.bounciness),
		mushyness(props.mushyness)
	{}

	BodyTrait::Body body;
	float mass;
	float bounciness;
	float mushyness;

	Vec2 vel{};
	Vec2 force{};
	bool onGround = false;

	void friction(Vec2 coef);
	void gravity(Vec2 g = Vec2(0, 20));

	void standardForces()
	{
		if (onGround) {
			friction(Vec2{1000, 100});
		}
		else {
			friction(Vec2{100, 100});
		}
		gravity();
	}

	void applyForce(Vec2 f) override;
	void addVelocity(Vec2 v) override;

	Vec2 velocity() override
	{
		return vel;
	}

	void collideWith(BodyTrait::Body &otehr);
	void collideAll(WorldPlane &plane);

	void update(const Context &ctx, float dt);

	void serialize(sbon::Writer w);
	void deserialize(sbon::Reader r);
};

/*
 * BasicPhysics
 */

inline void BasicPhysicsBody::friction(Vec2 coef)
{
	force += -vel * coef;
}

inline void BasicPhysicsBody::gravity(Vec2 g)
{
	force += g * mass;
}

inline void BasicPhysicsBody::applyForce(Vec2 f)
{
	force += f;
}

inline void BasicPhysicsBody::addVelocity(Vec2 v)
{
	vel += v;
}

}
