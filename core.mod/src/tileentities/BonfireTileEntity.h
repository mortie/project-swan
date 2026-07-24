#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"

namespace CoreMod {

class BonfireTileEntity final: public Swan::Entity,
	public Swan::TileEntityTrait {
public:
	using Proto = proto::BonfireTileEntity;

	BonfireTileEntity(Swan::Ctx &)
	{}

	TileEntity &get(TileEntityTrait::Tag) override
	{
		return tileEntity_;
	}

	void update(Swan::Ctx &ctx, float dt) override;
	void tick(Swan::Ctx &ctx, float dt) override;

	void tickBonfire(Swan::Ctx &ctx, float dt);
	void tickCrucible(Swan::Ctx &ctx, float dt);

	void serialize(Swan::Ctx &ctx, Proto::Builder w);
	void deserialize(Swan::Ctx &ctx, Proto::Reader r);

	void onDespawn(Swan::Ctx &ctx) override { evacuateCrucible(ctx); }

	void activateCrucible(Swan::Ctx &ctx, Swan::ItemStack &stack);
	void evacuateCrucible(Swan::Ctx &ctx);

private:
	struct OngoingBurn {
		std::vector<Swan::EntityRef> inputs;
		Swan::ItemStack output;
		float timer;
	};

	struct Crucible {
		struct Progress {
			float timer;
			Swan::ItemStack output;
		};

		std::vector<Swan::Item *> items;
		std::unordered_map<Swan::Item *, int> itemCounts;
		std::optional<Progress> progress;
	};

	TileEntity tileEntity_ = {
		.keep = true,
	};
	std::vector<OngoingBurn> ongoing_;
	Crucible crucible_;
};

}
