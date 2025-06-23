#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"

namespace CoreMod {

class CrucibleTileEntity final: public Swan::Entity,
	public Swan::TileEntityTrait {
public:
	using Proto = proto::CrucibleTileEntity;

	CrucibleTileEntity(const Swan::Context &ctx):
		sprite_(ctx.world.getSprite("core::misc/crucible-support"))
	{}

	TileEntity &get(TileEntityTrait::Tag) override
	{
		return tileEntity_;
	}

	void draw(const Swan::Context &ctx, Cygnet::Renderer &rnd) override;
	void tick(const Swan::Context &ctx, float dt) override;

	void serialize(const Swan::Context &ctx, Proto::Builder w);
	void deserialize(const Swan::Context &ctx, Proto::Reader r);

	bool drawSupports_ = false;
	float temperature_ = 0;

private:
	TileEntity tileEntity_;
	Cygnet::RenderSprite sprite_;
};

}
