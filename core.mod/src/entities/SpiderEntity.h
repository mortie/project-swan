#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"

namespace CoreMod {

class SpiderEntity final: public Swan::Entity,
	public Swan::PhysicsBodyTrait, public Swan::ContactDamageTrait {
public:
	using Proto = proto::SpiderEntity;

	SpiderEntity(const Swan::Context &ctx);
	SpiderEntity(const Swan::Context &ctx, Swan::Vec2 pos);

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

	void draw(const Swan::Context &ctx, Cygnet::Renderer &rnd) override;
	void update(const Swan::Context &ctx, float dt) override;
	void tick(const Swan::Context &ctx, float dt) override;

	void serialize(const Swan::Context &ctx, Proto::Builder w);
	void deserialize(const Swan::Context &ctx, Proto::Reader r);

private:
	Swan::Animation idleAnimation_;

	float jumpTimer_ = 0;
	Body *target_ = nullptr;

	Swan::BasicPhysicsBody physicsBody_;
	Damage damage_{};
};

}
