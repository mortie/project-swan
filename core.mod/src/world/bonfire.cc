#include "bonfire.h"

#include "util.h"
#include "tileentities/BonfireTileEntity.h"
#include "tiles.h"

namespace CoreMod {

void registerBonfire(Swan::Mod &mod)
{
	mod.registerEntity<BonfireTileEntity>("tile::bonfire");

	mod.registerTile({
		.name = "bonfire",
		.image = "core::tiles/bonfire::unlit",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::bonfire",
		.onSpawn = [](Swan::Ctx &ctx, Swan::TilePos pos) {
			if (!denyIfFloating(ctx, pos)) {
				return false;
			}

			ctx.plane.tiles().setID(pos, tiles::bonfire__lit);
			return true;
		},
		.onTileUpdate = fallIfFloating,
		.onWorldTick = breakIfInFluid,
	});

	mod.registerTile({
		.name = "bonfire::lit",
		.image = "core::tiles/bonfire::lit",
		.isSolid = false,
		.lightLevel = 0.1,
		.temperature = 1,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::bonfire",
		.tileEntity = "core::tile::bonfire",
	});
}

}
