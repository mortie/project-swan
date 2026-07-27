#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"
#include "swan/EntityCollection.h"

namespace CoreMod {

class IncandescentLampTileEntity final: public Swan::Entity,
	public Swan::TileEntityTrait {
public:
	using Proto = proto::IncandescentLampTileEntity;

	IncandescentLampTileEntity(Swan::Ctx &ctx):
		glowSprite_(ctx.world.getSprite("core::misc/incandescent-lamp-glow")),
		glowRedSprite_(ctx.world.getSprite("core::misc/incandescent-lamp-glow-red"))
	{}

	Swan::TileEntity &get(Swan::TileEntityTrait::Tag) override
	{ return tileEntity_; }

	void onSpawn(Swan::Ctx &ctx) override;
	void tick(Swan::Ctx &ctx, float dt) override;
	void draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd) override;
	void onDespawn(Swan::Ctx &ctx) override;
	void drawDebug(Swan::Ctx &ctx) override;

	void serialize(Swan::Ctx &ctx, Proto::Builder w);
	void deserialize(Swan::Ctx &ctx, Proto::Reader r);

private:
	float kelvin() { return temperature_ + 300; }

	Swan::TileEntity tileEntity_;
	Swan::EntityRef powerSource_;

	// Temperature, represented as "degrees kelvin over ambient".
	// "Ambient" is assumed to be 300k (~27C).
	// The numbers will be in the thousands before any light appears,
	// so the exact ambient temperature doesn't matter and 300k
	// is a nice round number.
	float temperature_ = 0;

	float light_ = 0;
	Cygnet::RenderSprite glowSprite_;
	Cygnet::RenderSprite glowRedSprite_;
};

}
