#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"

namespace CoreMod {

class AqueductTileEntity final: public Swan::Entity,
	public Swan::TileEntityTrait {
public:
	using Proto = proto::AqueductTileEntity;

	AqueductTileEntity(Swan::Ctx &)
	{}

	TileEntity &get(TileEntityTrait::Tag) override
	{
		return tileEntity_;
	}

	void onTileUpdate(Swan::Ctx &ctx);

	void tick(Swan::Ctx &ctx, float dt) override;
	void tick2(Swan::Ctx &ctx, float dt) override;

	void serialize(Swan::Ctx &ctx, Proto::Builder w);
	void deserialize(Swan::Ctx &ctx, Proto::Reader r);

private:
	struct FluidStack {
		float level = 0;
		Swan::Fluid *fluid = nullptr;
	};

	TileEntity tileEntity_{
		.keep = true,
	};

	FluidStack content_;
	float velLeft_;
	float velRight_;

	Swan::EntityRef left_;
	Swan::EntityRef right_;
};

}
