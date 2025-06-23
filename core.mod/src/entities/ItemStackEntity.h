#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"

namespace CoreMod {

class ItemStackEntity final: public Swan::Entity,
	public Swan::PhysicsBodyTrait {
public:
	using Proto = proto::ItemStackEntity;

	ItemStackEntity(Swan::Ctx &ctx);
	ItemStackEntity(Swan::Ctx &ctx, Swan::Vec2 pos, Swan::Item *item);
	ItemStackEntity(Swan::Ctx &ctx, Swan::Vec2 pos, Swan::Vec2 vel, Swan::Item *item);

	Body &get(BodyTrait::Tag) override
	{
		return physicsBody_.body;
	}

	PhysicsBody &get(PhysicsBodyTrait::Tag) override
	{
		return physicsBody_;
	}

	void draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd) override;
	void update(Swan::Ctx &ctx, float dt) override;
	void tick(Swan::Ctx &ctx, float dt) override;

	void serialize(Swan::Ctx &ctx, Proto::Builder w);
	void deserialize(Swan::Ctx &ctx, Proto::Reader r);

	Swan::Item *item()
	{
		return item_;
	}

	float lifetime_ = 0;

private:
	Swan::Item *item_;
	Swan::BasicPhysicsBody physicsBody_;
};

}
