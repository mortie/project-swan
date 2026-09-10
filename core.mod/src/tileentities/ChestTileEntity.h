#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"

namespace CoreMod {

class ChestTileEntity final: public Swan::Entity,
	public Swan::TileEntityTrait,
	public Swan::InventoryTrait {
public:
	ChestTileEntity(Swan::Ctx &ctx)
	{}

	Swan::TileEntity &get(TileEntityTrait::Tag) override
	{
		return tileEntity_;
	}

	Swan::Inventory &get(InventoryTrait::Tag) override
	{
		return inventory_;
	}

	void serialize(Swan::Ctx &ctx, capnp::MessageBuilder &mb) override;
	void deserialize(Swan::Ctx &ctx, capnp::MessageReader &mr) override;

	void onDespawn(Swan::Ctx &ctx) override;

private:
	Swan::TileEntity tileEntity_{
		.keep = true,
	};
	Swan::BasicInventory inventory_{30};
};

}
