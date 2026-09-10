#pragma once

#include <swan/swan.h>

namespace CoreMod {

class DrainTileEntity final: public Swan::Entity,
	public Swan::TileEntityTrait {
public:
	DrainTileEntity(Swan::Ctx &ctx)
	{}

	Swan::TileEntity &get(TileEntityTrait::Tag) override
	{
		return tileEntity_;
	}

	void tick(Swan::Ctx &ctx, float dt) override;

	void serialize(Swan::Ctx &ctx, capnp::MessageBuilder &mb) override;
	void deserialize(Swan::Ctx &ctx, capnp::MessageReader &mr) override;

private:
	Swan::TileEntity tileEntity_;
	int counter_ = 0;
};

}
