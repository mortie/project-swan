#include "drain.h"

#include "tileentities/DrainTileEntity.h"

namespace CoreMod {

void registerDrain(Swan::Mod &mod)
{
	mod.registerEntity<DrainTileEntity>("tile::drain");

	mod.registerTile({
		.name = "drain",
		.image = "core::tiles/drain@0",
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::drain",
		.onActivate = +[](Swan::Ctx &ctx, Swan::TilePos pos, Swan::Tile::ActivateMeta) {
			ctx.plane.tiles().set(pos, "core::drain::open");
		},
	});

	mod.registerTile({
		.name = "drain::open",
		.image = "core::tiles/drain@1",
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::drain",
		.tileEntity = "core::tile::drain",
		.onActivate = +[](Swan::Ctx &ctx, Swan::TilePos pos, Swan::Tile::ActivateMeta) {
			ctx.plane.tiles().set(pos, "core::drain");
		},
	});
}

}
