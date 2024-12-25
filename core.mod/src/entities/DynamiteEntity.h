#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"

namespace CoreMod {

class DynamiteEntity final: public Swan::Entity,
	public Swan::PhysicsBodyTrait {
public:
	DynamiteEntity(
		const Swan::Context &ctx, Swan::Vec2 pos, Swan::Vec2 vel = {0, 0});

	DynamiteEntity(const Swan::Context &ctx):
		tile_(ctx.world.getItem("core::dynamite").id)
	{}

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

	void serialize(const Swan::Context &ctx, sbon::ObjectWriter w) override;
	void deserialize(const Swan::Context &ctx, sbon::ObjectReader r) override;

private:
	static constexpr Swan::BasicPhysicsBody::Props PROPS = {
		.size = {0.8, 0.2},
		.mass = 20,
		.isSolid = false,
	};
	static constexpr float FUSE_TIME = 5;

	Swan::Tile::ID tile_;
	float fuse_ = FUSE_TIME;
	Swan::BasicPhysicsBody physicsBody_{PROPS};
};

}
