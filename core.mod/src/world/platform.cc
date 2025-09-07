#include "platform.h"

#include <memory>

#include "util.h"

namespace CoreMod {

static bool spawnPlatform(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto &left = ctx.plane.tiles().get(pos.add(-1, 0));
	auto &right = ctx.plane.tiles().get(pos.add(1, 0));
	if (left.isSupportH() || right.isSupportH()) {
		return true;
	}

	auto &below = ctx.plane.tiles().get(pos.add(0, 1));
	return below.isSolid();
}

static void updatePlatform(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto &below = ctx.plane.tiles().get(pos.add(0, 1));
	auto &left = ctx.plane.tiles().get(pos.add(-1, 0));
	auto &right = ctx.plane.tiles().get(pos.add(1, 0));

	if (!below.isSolid() && !left.isSupportH() && !right.isSupportH()) {
		breakTileAndDropItem(ctx, pos);
		return;
	}

	auto &self = ctx.plane.tiles().get(pos);
	bool hasSolidBelow = below.isSolid();
	bool hasPlatformLeft = left.more->traits == self.more->traits;
	bool hasSolidLeft = left.isSupportH() && !hasPlatformLeft;
	bool hasPlatformRight = right.more->traits == self.more->traits;
	bool hasSolidRight = right.isSupportH() && !hasPlatformRight;

	std::string_view desired = "core::platform::lone";
	if (hasSolidBelow) {
		if (hasPlatformLeft && hasPlatformRight) {
			desired = "core::platform::supported::center";
		} else if (hasPlatformLeft) {
			desired = "core::platform::supported::right";
		} else if (hasPlatformRight) {
			desired = "core::platform::supported::left";
		} else {
			desired = "core::platform";
		}
	} else {
		if (hasPlatformLeft && hasPlatformRight) {
			desired = "core::platform::center";
		} else if (hasPlatformLeft && hasSolidRight) {
			desired = "core::platform::anchored::right";
		} else if (hasSolidLeft && hasPlatformRight) {
			desired = "core::platform::anchored::left";
		} else if (hasPlatformLeft) {
			desired = "core::platform::right";
		} else if (hasPlatformRight) {
			desired = "core::platform::left";
		} else if (hasSolidLeft) {
			desired = "core::platform::anchored::stub::right";
		} else if (hasSolidRight) {
			desired = "core::platform::anchored::stub::left";
		}
	}

	if (self.name != desired) {
		ctx.plane.tiles().set(pos, desired);
	}
}

void registerPlatform(Swan::Mod &mod)
{
	auto traits = std::make_shared<Swan::Tile::Traits>();

	Swan::Tile::Builder tile = {
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::platform",
		.isSolid = false,
		.isPlatform = true,
		.isSupportV = true,
		.isSupportH = true,
		.onTileUpdate = updatePlatform,
		.onSpawn = spawnPlatform,
		.traits = traits,
	};

	mod.registerTile(tile
		.withName("platform::left")
		.withImage("core::tiles/platform@0"));
	mod.registerTile(tile
		.withName("platform::right")
		.withImage("core::tiles/platform@2"));
	mod.registerTile(tile
		.withName("platform::center")
		.withImage("core::tiles/platform@1"));

	mod.registerTile(tile
		.withName("platform::anchored::stub::left")
		.withImage("core::tiles/platform@3"));
	mod.registerTile(tile
		.withName("platform::anchored::stub::right")
		.withImage("core::tiles/platform@5"));
	mod.registerTile(tile
		.withName("platform::lone")
		.withImage("core::tiles/platform@4"));

	mod.registerTile(tile
		.withName("platform::anchored::left")
		.withImage("core::tiles/platform@6"));
	mod.registerTile(tile
		.withName("platform::anchored::right")
		.withImage("core::tiles/platform@8"));
	mod.registerTile(tile
		.withName("platform")
		.withImage("core::tiles/platform@7"));

	mod.registerTile(tile
		.withName("platform::supported::left")
		.withImage("core::tiles/platform@9"));
	mod.registerTile(tile
		.withName("platform::supported::right")
		.withImage("core::tiles/platform@11"));
	mod.registerTile(tile
		.withName("platform::supported::center")
		.withImage("core::tiles/platform@10"));
}

}
