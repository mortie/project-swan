#pragma once

#include <swan/swan.h>

namespace CoreMod {

class ItemFanTileEntity final: public Swan::Entity,
	public Swan::TileEntityTrait {
public:
	ItemFanTileEntity(const Swan::Context &ctx)
	{}

	TileEntity &get(TileEntityTrait::Tag) override
	{
		return tileEntity_;
	}

	void update(const Swan::Context &ctx, float dt) override;
	void tick(const Swan::Context &ctx, float dt) override;

	void setDirection(Swan::Direction dir)
	{
		dir_ = dir;
	}

	void serialize(const Swan::Context &ctx, sbon::ObjectWriter w) override;
	void deserialize(const Swan::Context &ctx, sbon::ObjectReader r) override;

private:
	Swan::Direction dir_;
	TileEntity tileEntity_;

	Swan::EntityRef pickup_;
};

}
