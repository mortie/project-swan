#pragma once

#include <swan/swan.h>
#include <unordered_map>

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

	void onDespawn(const Swan::Context &ctx) override;

	void activate(const Swan::Context &ctx, Swan::ItemStack &stack);

	bool drawSupports_ = false;
	float temperature_ = 0;

private:
	struct Progress {
		float timer;
		Swan::ItemStack output;
	};

	TileEntity tileEntity_;
	Cygnet::RenderSprite sprite_;
	std::vector<Swan::Item *> items_;
	std::unordered_map<Swan::Item *, int> itemCounts_;
	std::optional<Progress> progress_;
};

}
