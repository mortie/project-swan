#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"

namespace CoreMod {

class SpiderEntity final: public Swan::Entity,
	public Swan::PhysicsBodyTrait, public Swan::ContactDamageTrait {
public:
	using Proto = proto::SpiderEntity;

	SpiderEntity(Swan::Ctx &ctx);
	SpiderEntity(Swan::Ctx &ctx, Swan::Vec2 pos);

	Body &get(BodyTrait::Tag) override
	{
		return physicsBody_.body;
	}

	PhysicsBody &get(PhysicsBodyTrait::Tag) override
	{
		return physicsBody_;
	}

	Damage &get(ContactDamageTrait::Tag) override
	{
		return damage_;
	}

	void draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd) override;
	void update(Swan::Ctx &ctx, float dt) override;
	void tick(Swan::Ctx &ctx, float dt) override;

	void serialize(Swan::Ctx &ctx, Proto::Builder w);
	void deserialize(Swan::Ctx &ctx, Proto::Reader r);

private:
	Swan::Animation idleAnimation_;

	float jumpTimer_ = 0;
	Body *target_ = nullptr;

	Swan::BasicPhysicsBody physicsBody_;
	Damage damage_{};
};

}
