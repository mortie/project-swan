#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"

namespace CoreMod {

class DynamiteEntity final: public Swan::Entity,
	public Swan::PhysicsBodyTrait {
public:
	using Proto = proto::DynamiteEntity;

	DynamiteEntity(const Swan::Context &ctx);
	DynamiteEntity(
		const Swan::Context &ctx, Swan::Vec2 pos, Swan::Vec2 vel = {0, 0});

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

	void serialize(const Swan::Context &ctx, Proto::Builder w);
	void deserialize(const Swan::Context &ctx, Proto::Reader r);

private:
	Swan::Tile::ID tile_;
	float fuse_;
	Swan::BasicPhysicsBody physicsBody_;
};

}
