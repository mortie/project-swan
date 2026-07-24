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
		.onTileUpdate = fallIfFloating,
		.onActivate = [](Swan::Ctx &ctx, Swan::TilePos pos, Swan::Tile::ActivateMeta meta) {
			if (meta.stack.empty()) {
				return false;
			}
			if (meta.stack.item()->name != "core::crucible") {
				return false;
			}

			meta.stack.remove(1);
			ctx.plane.tiles().setID(pos, tiles::bonfire__crucibled);
			ctx.game.playSound(ctx.world.getSound("@::thud"));
			return true;
		},
		.onWorldTick = breakIfInFluid,
	});

	mod.registerTile({
		.name = "bonfire::crucibled",
		.image = "core::tiles/bonfire::crucibled",
		.isSolid = false,
		.lightLevel = 0.1,
		.temperature = 1,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::bonfire",
		.tileEntity = "core::tile::bonfire",
		.onBreak = [](Swan::Ctx &ctx, Swan::TilePos pos) {
			dropItem(ctx, pos, "core::crucible");
		},
		.onTileUpdate = fallIfFloating,
		.onActivate = [](Swan::Ctx &ctx, Swan::TilePos pos, Swan::Tile::ActivateMeta meta) {
			if (meta.stack.empty()) {
				ctx.game.playSound(ctx.world.getSound("@::thud"));
				ctx.plane.tiles().setID(pos, tiles::bonfire__lit);
				auto ent = ctx.plane.entities().getTileEntity(pos).as<BonfireTileEntity>();
				if (ent) {
					ent->evacuateCrucible(ctx);
				}

				return true;
			}

			auto ent = ctx.plane.entities().getTileEntity(pos).as<BonfireTileEntity>();
			if (!ent) {
				return false;
			}

			ent->activateCrucible(ctx, meta.stack);
			return true;
		},
		.onWorldTick = breakIfInFluid,
	});
}

}
