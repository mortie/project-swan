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

	void tick(Swan::Ctx &ctx, float dt) override;

	void serialize(Swan::Ctx &ctx, Proto::Builder w);
	void deserialize(Swan::Ctx &ctx, Proto::Reader r);

private:
	struct OngoingBurn {
		std::vector<Swan::EntityRef> inputs;
		Swan::ItemStack output;
		float timer;
	};

	TileEntity tileEntity_;
	std::vector<OngoingBurn> ongoing_;
};

}
