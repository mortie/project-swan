#include "crucible.h"

#include "util.h"
#include "tileentities/CrucibleTileEntity.h"

namespace CoreMod {

static bool isSupported(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto &below = ctx.plane.tiles().get(pos.add(0, 1));
	if (below.isSupportV) {
		return true;
	}

	auto &below2 = ctx.plane.tiles().get(pos.add(0, 2));
	if (!below2.isSolid) {
		return false;
	}

	if (below.temperature > 0 && !below.isOpaque) {
		return true;
	}

	return false;
}

void registerCrucible(Swan::Mod &mod)
{
	mod.registerEntity<CrucibleTileEntity>("tile::crucible");

	mod.registerTile({
		.name = "crucible",
		.image = "core::tiles/crucible",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::crucible",
		.tileEntity = "core::tile::crucible",
		.onSpawn = isSupported,
		.onTileUpdate = +[](Swan::Ctx &ctx, Swan::TilePos pos) {
			if (!isSupported(ctx, pos)) {
				breakTileAndDropItem(ctx, pos);
				return;
			}

			auto ent = ctx.plane.entities().getTileEntity(pos).as<CrucibleTileEntity>();
			if (ent) {
				auto below = ctx.plane.tiles().get(pos.add(0, 1));
				ent->drawSupports_ = !below.isSupportV;
				ent->temperature_ = below.temperature;
			}
		},
		.onActivate = +[](Swan::Ctx &ctx, Swan::TilePos pos, Swan::Tile::ActivateMeta meta) {
			auto ent = ctx.plane.entities().getTileEntity(pos).as<CrucibleTileEntity>();
			if (ent) {
				ent->activate(ctx, meta.stack);
			}
		},
	});
}

}
