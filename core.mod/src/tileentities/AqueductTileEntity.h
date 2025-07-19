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

	void draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd) override;
	void tick(Swan::Ctx &ctx, float dt) override;
	void tick2(Swan::Ctx &ctx, float dt) override;
	void drawDebug(Swan::Ctx &ctx) override;
	void onWorldLoaded(Swan::Ctx &ctx) override
	{
		onTileUpdate(ctx);
	}

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
	float dropAcc_;

	// levelSnapshot_ contains the amount of fluid in the aqueduct after a tick().
	// It is used by tick2() as the basis for how much fluid to move.
	float levelSnapshot_;

	Swan::EntityRef left_;
	Swan::EntityRef right_;
	Swan::DirectionSet connectedTo_;
};

}
