#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"

namespace CoreMod {

class ChestTileEntity final: public Swan::Entity,
	public Swan::TileEntityTrait,
	public Swan::InventoryTrait {
public:
	using Proto = proto::ChestTileEntity;

	ChestTileEntity(Swan::Ctx &ctx)
	{}

	TileEntity &get(TileEntityTrait::Tag) override
	{
		return tileEntity_;
	}

	Inventory &get(InventoryTrait::Tag) override
	{
		return inventory_;
	}

	void serialize(Swan::Ctx &ctx, Proto::Builder w);
	void deserialize(Swan::Ctx &ctx, Proto::Reader r);

	void onDespawn(Swan::Ctx &ctx) override;

private:
	TileEntity tileEntity_{
		.keep = true,
	};
	Swan::BasicInventory inventory_{30};
};

}
