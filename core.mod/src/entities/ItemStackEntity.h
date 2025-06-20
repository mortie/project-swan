#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"

namespace CoreMod {

class ItemStackEntity final: public Swan::Entity,
	public Swan::PhysicsBodyTrait {
public:
	using Proto = proto::ItemStackEntity;

	ItemStackEntity(const Swan::Context &ctx);
	ItemStackEntity(const Swan::Context &ctx, Swan::Vec2 pos, Swan::Item *item);
	ItemStackEntity(const Swan::Context &ctx, Swan::Vec2 pos, Swan::Vec2 vel, Swan::Item *item);

	Body &get(BodyTrait::Tag) override
	{
		return physicsBody_.body;
	}

	PhysicsBody &get(PhysicsBodyTrait::Tag) override
	{
		return physicsBody_;
	}

	void draw(const Swan::Context &ctx, Cygnet::Renderer &rnd) override;
	void update(const Swan::Context &ctx, float dt) override;
	void tick(const Swan::Context &ctx, float dt) override;

	void serialize(const Swan::Context &ctx, Proto::Builder w);
	void deserialize(const Swan::Context &ctx, Proto::Reader r);

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
