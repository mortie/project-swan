#include "DrainTileEntity.h"

namespace CoreMod {

void DrainTileEntity::tick(Swan::Ctx &ctx, float dt)
{
	counter_ += 1;
	if (counter_ >= 2) {
		counter_ = 0;
		Swan::Fluid &fluid = ctx.plane.fluids().takeAnyFromRow(
			tileEntity_.pos.add(0, -1), Swan::FLUID_RESOLUTION - 1);
		if (fluid.id != Swan::World::AIR_FLUID_ID) {
			ctx.plane.fluids().spawnFluidParticle(
				tileEntity_.pos.as<float>().add(0.5 - 1.0/8.0, 1), fluid.id);
		}
	}
}

void DrainTileEntity::serialize(Swan::Ctx &ctx, Proto::Builder w)
{
	tileEntity_.serialize(w);
}

void DrainTileEntity::deserialize(Swan::Ctx &ctx, Proto::Reader r)
{
	tileEntity_.deserialize(r);
}

}
