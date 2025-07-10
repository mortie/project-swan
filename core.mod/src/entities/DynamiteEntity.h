#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"

namespace CoreMod {

class DynamiteEntity final: public Swan::Entity,
	public Swan::PhysicsBodyTrait {
public:
	using Proto = proto::DynamiteEntity;

	DynamiteEntity(Swan::Ctx &ctx);
	DynamiteEntity(
		Swan::Ctx &ctx, Swan::Vec2 pos, Swan::Vec2 vel = {0, 0});

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

private:
	Swan::Animation animation_;

	Swan::SoundHandle fuseSound_;
	float fuse_;
	Swan::BasicPhysicsBody physicsBody_;
};

}
