#include "outcrop.h"

#include "util.h"

namespace CoreMod {

static bool onOutcropSpawn(Swan::Ctx &ctx, Swan::TilePos pos)
{
	// The default outcrop stands on the tile below it,
	// so if that's valid, we're good
	if (ctx.plane.tiles().get(pos.add(0, 1)).isSupportV()) {
		return true;
	}

	auto &tile = ctx.plane.tiles().get(pos);

	if (ctx.plane.tiles().get(pos.add(-1, 0)).isSupportH()) {
		ctx.plane.tiles().set(pos, Swan::cat(tile.name, "::left"));
		return true;
	}
	else if (ctx.plane.tiles().get(pos.add(1, 0)).isSupportH()) {
		ctx.plane.tiles().set(pos, Swan::cat(tile.name, "::right"));
		return true;
	}
	else if (ctx.plane.tiles().get(pos.add(0, -1)).isSupportV()) {
		ctx.plane.tiles().set(pos, Swan::cat(tile.name, "::hanging"));
		return true;
	}

	return false;
}

static void onOutcropUpdate(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto &tile = ctx.plane.tiles().get(pos);
	bool isSupported;

	if (tile.name.str().ends_with("::left")) {
		isSupported = ctx.plane.tiles().get(pos.add(-1, 0)).isSupportH();
	}
	else if (tile.name.str().ends_with("::right")) {
		isSupported = ctx.plane.tiles().get(pos.add(1, 0)).isSupportH();
	}
	else if (tile.name.str().ends_with("::hanging")) {
		isSupported = ctx.plane.tiles().get(pos.add(0, -1)).isSupportV();
	}
	else {
		isSupported = ctx.plane.tiles().get(pos.add(0, 1)).isSupportV();
	}

	if (!isSupported) {
		breakTileAndDropItem(ctx, pos);
	}
}

void registerOutcrop(Swan::Mod &mod, Swan::Tile::Builder builder)
{
	builder
		.withIsSolid(false)
		.withIsSupportH(false)
		.withIsSupportV(false)
		.withBreakableBy(Swan::Tool::HAND)
		.withOnTileUpdate(onOutcropUpdate)
		.withIsOpaque(false);

	mod.registerTile(builder
		.clone()
		.withImage(Swan::cat(builder.image, "::normal"))
		.withOnSpawn(onOutcropSpawn));

	mod.registerTile(builder
		.clone()
		.withName(Swan::cat(builder.name, "::hanging"))
		.withImage(Swan::cat(builder.image, "::hanging")));

	mod.registerTile(builder
		.clone()
		.withName(Swan::cat(builder.name, "::left"))
		.withImage(Swan::cat(builder.image, "::left")));

	mod.registerTile(builder
		.clone()
		.withName(Swan::cat(builder.name, "::right"))
		.withImage(Swan::cat(builder.image, "::right")));
}

}
