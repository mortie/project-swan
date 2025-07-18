#include "outcrop.h"

#include "util.h"

namespace CoreMod {

static bool onOutcropSpawn(Swan::Ctx &ctx, Swan::TilePos pos)
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
	else if (ctx.plane.tiles().get(pos.add(0, -1)).isSupportV) {
		ctx.plane.tiles().set(pos, Swan::cat(tile.name, "::hanging"));
		return true;
	}

	return false;
}

static void onOutcropUpdate(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto &tile = ctx.plane.tiles().get(pos);
	bool isSupported;

	if (tile.name.ends_with("::left")) {
		isSupported = ctx.plane.tiles().get(pos.add(-1, 0)).isSupportH;
	}
	else if (tile.name.ends_with("::right")) {
		isSupported = ctx.plane.tiles().get(pos.add(1, 0)).isSupportH;
	}
	else if (tile.name.ends_with("::hanging")) {
		isSupported = ctx.plane.tiles().get(pos.add(0, -1)).isSupportV;
	}
	else {
		isSupported = ctx.plane.tiles().get(pos.add(0, 1)).isSupportV;
	}

	if (!isSupported) {
		breakTileAndDropItem(ctx, pos);
	}
}

void registerOutcrop(Swan::Mod &mod, const char *name, const char *item)
{
	if (item == nullptr) {
		item = name;
	}

	mod.registerTile({
		.name = Swan::cat(name, "-outcrop"),
		.image = Swan::cat("core::tiles/geo/", name, "-outcrop::normal"),
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = Swan::cat("core::", item),
		.onSpawn = onOutcropSpawn,
		.onTileUpdate = onOutcropUpdate,
	});

	mod.registerTile({
		.name = Swan::cat(name, "-outcrop::hanging"),
		.image = Swan::cat("core::tiles/geo/", name, "-outcrop::hanging"),
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = Swan::cat("core::", item),
		.onTileUpdate = onOutcropUpdate,
	});

	mod.registerTile({
		.name = Swan::cat(name, "-outcrop::left"),
		.image = Swan::cat("core::tiles/geo/", name, "-outcrop::left"),
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = Swan::cat("core::", item),
		.onTileUpdate = onOutcropUpdate,
	});

	mod.registerTile({
		.name = Swan::cat(name, "-outcrop::right"),
		.image = Swan::cat("core::tiles/geo/", name, "-outcrop::right"),
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = Swan::cat("core::", item),
		.onTileUpdate = onOutcropUpdate,
	});
}

}
