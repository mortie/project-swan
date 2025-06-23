#include "bonfire.h"

#include "util.h"
#include "tileentities/BonfireTileEntity.h"

namespace CoreMod {

void registerBonfire(Swan::Mod &mod)
{
	mod.registerEntity<BonfireTileEntity>("tile::bonfire");

	mod.registerTile({
		.name = "bonfire",
		.image = "core::tiles/bonfire",
		.isSolid = false,
		.lightLevel = 0.1,
		.temperature = 1,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::bonfire",
		.tileEntity = "core::tile::bonfire",
		.onSpawn = denyIfFloating,
		.onTileUpdate = fallIfFloating,
	});
}

}
