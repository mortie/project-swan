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

private:
	Swan::Direction dir_;
	TileEntity tileEntity_;
};

}
