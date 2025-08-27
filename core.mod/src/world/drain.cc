#include "drain.h"

#include "tileentities/DrainTileEntity.h"

namespace CoreMod {

void registerDrain(Swan::Mod &mod)
{
	mod.registerEntity<DrainTileEntity>("tile::drain");

	auto fluidCollision = std::make_shared<Swan::FluidCollision>(
		0b1111'1001'1001'0000);

	mod.registerTile({
		.name = "drain",
		.image = "core::tiles/drain@0",
		.fluidMask = "core::masks/drain",
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::drain",
		.onActivate = +[](Swan::Ctx &ctx, Swan::TilePos pos, Swan::Tile::ActivateMeta) {
			ctx.plane.tiles().set(pos, "core::drain::open");
		},
		.fluidCollision = fluidCollision,
	});

	mod.registerTile({
		.name = "drain::open",
		.image = "core::tiles/drain@1",
		.fluidMask = "core::masks/drain",
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::drain",
		.tileEntity = "core::tile::drain",
		.onActivate = +[](Swan::Ctx &ctx, Swan::TilePos pos, Swan::Tile::ActivateMeta) {
			ctx.plane.tiles().set(pos, "core::drain");
		},
		.fluidCollision = fluidCollision,
	});
}

}
