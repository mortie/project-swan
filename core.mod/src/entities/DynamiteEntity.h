#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"

namespace CoreMod {

class DynamiteEntity final: public Swan::Entity,
	public Swan::PhysicsBodyTrait {
public:
	DynamiteEntity(Swan::Ctx &ctx);
	DynamiteEntity(
		Swan::Ctx &ctx, Swan::Vec2 pos, Swan::Vec2 vel = {0, 0});

	Swan::Body &get(BodyTrait::Tag) override
	{
		return physicsBody_.body;
	}

	Swan::PhysicsBody &get(PhysicsBodyTrait::Tag) override
	{
		return physicsBody_;
	}

	void draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd) override;
	void update(Swan::Ctx &ctx, float dt) override;
	void tick(Swan::Ctx &ctx, float dt) override;

	void serialize(Swan::Ctx &ctx, capnp::MessageBuilder &mb) override;
	void deserialize(Swan::Ctx &ctx, capnp::MessageReader &mr) override;

private:
	Swan::Animation animation_;

	Swan::SoundHandle fuseSound_;
	float fuse_;
	Swan::BasicPhysicsBody physicsBody_;
};

}
