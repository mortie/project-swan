#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"

namespace CoreMod {

class ItemFanTileEntity final: public Swan::Entity,
	public Swan::TileEntityTrait {
public:
	using Proto = proto::ItemFanTileEntity;

	ItemFanTileEntity(Swan::Ctx &ctx)
	{}

	TileEntity &get(TileEntityTrait::Tag) override
	{
		return tileEntity_;
	}

	void update(Swan::Ctx &ctx, float dt) override;
	void tick(Swan::Ctx &ctx, float dt) override;

	void setDirection(Swan::Direction dir)
	{
		dir_ = dir;
	}

	void serialize(Swan::Ctx &ctx, Proto::Builder w);
	void deserialize(Swan::Ctx &ctx, Proto::Reader r);

private:
	Swan::Direction dir_;
	TileEntity tileEntity_;

	Swan::EntityRef pickup_;
};

}
