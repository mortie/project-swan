#pragma once

#include <swan/swan.h>
#include <unordered_map>

#include "core_mod.capnp.h"

namespace CoreMod {

class CrucibleTileEntity final: public Swan::Entity,
	public Swan::TileEntityTrait {
public:
	using Proto = proto::CrucibleTileEntity;

	CrucibleTileEntity(Swan::Ctx &ctx):
		sprite_(ctx.world.getSprite("core::misc/crucible-support"))
	{}

	TileEntity &get(TileEntityTrait::Tag) override
	{
		return tileEntity_;
	}

	void draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd) override;
	void tick(Swan::Ctx &ctx, float dt) override;

	void serialize(Swan::Ctx &ctx, Proto::Builder w);
	void deserialize(Swan::Ctx &ctx, Proto::Reader r);

	void onDespawn(Swan::Ctx &ctx) override;

	void activate(Swan::Ctx &ctx, Swan::ItemStack &stack);

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
