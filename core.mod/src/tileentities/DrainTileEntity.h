#pragma once

#include <swan/swan.h>

namespace CoreMod {

class DrainTileEntity final: public Swan::Entity,
	public Swan::TileEntityTrait {
public:
	using Proto = Swan::proto::TileEntity;

	DrainTileEntity(Swan::Ctx &ctx)
	{}

	TileEntity &get(TileEntityTrait::Tag) override
	{
		return tileEntity_;
	}

	void tick(Swan::Ctx &ctx, float dt) override;

	void serialize(Swan::Ctx &ctx, Proto::Builder w);
	void deserialize(Swan::Ctx &ctx, Proto::Reader r);

private:
	TileEntity tileEntity_;
	int counter_ = 0;
};

}
