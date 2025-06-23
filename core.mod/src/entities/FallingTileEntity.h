#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"

namespace CoreMod {

class FallingTileEntity final: public Swan::Entity,
	public Swan::PhysicsBodyTrait {
public:
	using Proto = proto::FallingTileEntity;

	FallingTileEntity(Swan::Ctx &ctx);
	FallingTileEntity(Swan::Ctx &ctx, Swan::Vec2 pos, Swan::Tile::ID tile);

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

	void serialize(Swan::Ctx &ctx, Proto::Builder w);
	void deserialize(Swan::Ctx &ctx, Proto::Reader r);

private:
	Swan::Tile::ID tile_ = Swan::World::INVALID_TILE_ID;
	Swan::BasicPhysicsBody physicsBody_;
};

}
