#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"

namespace CoreMod {

class SpiderEntity final: public Swan::Entity,
	public Swan::PhysicsBodyTrait,
	public Swan::ContactDamageTrait {
public:
	SpiderEntity(Swan::Ctx &ctx);
	SpiderEntity(Swan::Ctx &ctx, Swan::Vec2 pos);

	Swan::Body &get(BodyTrait::Tag) override
	{
		return physicsBody_.body;
	}

	Swan::PhysicsBody &get(PhysicsBodyTrait::Tag) override
	{
		return physicsBody_;
	}

	Swan::ContactDamage get(ContactDamageTrait::Tag) override
	{
		return {};
	}

	void draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd) override;
	void update(Swan::Ctx &ctx, float dt) override;
	void tick(Swan::Ctx &ctx, float dt) override;

	void serialize(Swan::Ctx &ctx, capnp::MessageBuilder &mb) override;
	void deserialize(Swan::Ctx &ctx, capnp::MessageReader &mr) override;

private:
	Swan::Animation idleAnimation_;

	float jumpTimer_ = 0;
	Swan::Body *target_ = nullptr;

	Swan::BasicPhysicsBody physicsBody_;
};

}
