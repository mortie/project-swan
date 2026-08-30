#include "DrainTileEntity.h"

namespace CoreMod {

void DrainTileEntity::tick(Swan::Ctx &ctx, float dt)
{
	counter_ += 1;
	if (counter_ >= 2) {
		counter_ = 0;
		Swan::Fluid &fluid = ctx.plane.fluids().takeAnyFromRow(
			tileEntity_.pos, 2);
		if (fluid.id != Swan::WorldData::AIR_FLUID_ID) {
			ctx.plane.fluids().spawnFluidParticle(
				tileEntity_.pos.as<float>().add(0.5 - 1.0/8.0, 1), fluid.id);
		}
	}
}

void DrainTileEntity::serialize(Swan::Ctx &ctx, capnp::MessageBuilder &mb)
{
	auto w = mb.initRoot<Swan::proto::TileEntity>();
	tileEntity_.serialize(w);
}

void DrainTileEntity::deserialize(Swan::Ctx &ctx, capnp::MessageReader &mr)
{
	auto r = mr.getRoot<Swan::proto::TileEntity>();
	tileEntity_.deserialize(r);
}

}
