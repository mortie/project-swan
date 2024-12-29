#include "outcrop.h"

#include "util.h"

namespace CoreMod {

static bool onOutcropSpawn(const Swan::Context &ctx, Swan::TilePos pos)
{
	// The default outcrop stands on the tile below it,
	// so if that's valid, we're good
	if (ctx.plane.tiles().get(pos.add(0, 1)).isSupportV) {
		return true;
	}

	auto &tile = ctx.plane.tiles().get(pos);

	if (ctx.plane.tiles().get(pos.add(-1, 0)).isSupportH) {
		ctx.plane.tiles().set(pos, Swan::cat(tile.name, "::left"));
		return true;
	}
	else if (ctx.plane.tiles().get(pos.add(1, 0)).isSupportH) {
		ctx.plane.tiles().set(pos, Swan::cat(tile.name, "::right"));
		return true;
	}

	return false;
}

static void onOutcropUpdate(const Swan::Context &ctx, Swan::TilePos pos)
{
	auto &tile = ctx.plane.tiles().get(pos);
	bool isSupported;

	if (tile.name.ends_with("::left")) {
		isSupported = ctx.plane.tiles().get(pos.add(-1, 0)).isSupportH;
	}
	else if (tile.name.ends_with("::right")) {
		isSupported = ctx.plane.tiles().get(pos.add(1, 0)).isSupportH;
	}
	else {
		isSupported = ctx.plane.tiles().get(pos.add(0, 1)).isSupportV;
	}

	if (!isSupported) {
		breakTileAndDropItem(ctx, pos);
	}
}

void registerOutcrop(Swan::Mod &mod, const char *name)
{
	mod.registerTile({
		.name = Swan::cat(name, "-outcrop"),
		.image = Swan::cat("core::tiles/outcrops/", name, "/normal"),
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = Swan::cat("core::", name),
		.onSpawn = onOutcropSpawn,
		.onTileUpdate = onOutcropUpdate,
	});

	mod.registerTile({
		.name = Swan::cat(name, "-outcrop::left"),
		.image = Swan::cat("core::tiles/outcrops/", name, "/wall::left"),
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = Swan::cat("core::", name),
		.onTileUpdate = onOutcropUpdate,
	});

	mod.registerTile({
		.name = Swan::cat(name, "-outcrop::right"),
		.image = Swan::cat("core::tiles/outcrops/", name, "/wall::right"),
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = Swan::cat("core::", name),
		.onTileUpdate = onOutcropUpdate,
	});
}

}
