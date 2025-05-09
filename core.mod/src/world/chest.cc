#include "chest.h"

#include "tileentities/ChestTileEntity.h"

namespace CoreMod {

void registerChest(Swan::Mod &mod)
{
	mod.registerEntity<ChestTileEntity>("tile::chest");

	mod.registerTile({
		.name = "chest",
		.image = "core::tiles/chest::closed",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::chest",
		.tileEntity = "core::tile::chest",
		.onActivate = +[](const Swan::Context &ctx, Swan::TilePos pos, Swan::EntityRef) {
			ctx.game.playSound(ctx.world.getSound("core::sounds/misc/lock-open"));
			ctx.plane.tiles().set(pos, "core::chest::open");
		},
	});
	mod.registerTile({
		.name = "chest::open",
		.image = "core::tiles/chest::open",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::chest",
		.tileEntity = "core::tile::chest",
		.onActivate = +[](const Swan::Context &ctx, Swan::TilePos pos, Swan::EntityRef) {
			ctx.game.playSound(ctx.world.getSound("core::sounds/misc/lock-close"));
			ctx.plane.tiles().set(pos, "core::chest");
		},
	});
}

}
