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

	std::string_view desired = "core::platform";
	if (hasSolidBelow) {
		if (hasPlatformLeft && hasPlatformRight) {
			desired = "core::platform::supported::center";
		} else if (hasPlatformLeft) {
			desired = "core::platform::supported::right";
		} else if (hasPlatformRight) {
			desired = "core::platform::supported::left";
		} else {
			desired = "core::platform::supported::lone";
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

	mod.registerTile({
		.name = "platform::left",
		.image = "core::tiles/platform@0",
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::platform",
		.isSolid = false,
		.isPlatform = true,
		.isSupportV = true,
		.isSupportH = true,
		.onTileUpdate = updatePlatform,
		.onSpawn = spawnPlatform,
		.traits = traits,
	});
	mod.registerTile({
		.name = "platform::right",
		.image = "core::tiles/platform@2",
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::platform",
		.isSolid = false,
		.isPlatform = true,
		.isSupportV = true,
		.isSupportH = true,
		.onTileUpdate = updatePlatform,
		.onSpawn = spawnPlatform,
		.traits = traits,
	});
	mod.registerTile({
		.name = "platform::center",
		.image = "core::tiles/platform@1",
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::platform",
		.isSolid = false,
		.isPlatform = true,
		.isSupportV = true,
		.isSupportH = true,
		.onTileUpdate = updatePlatform,
		.onSpawn = spawnPlatform,
		.traits = traits,
	});

	mod.registerTile({
		.name = "platform::anchored::stub::left",
		.image = "core::tiles/platform@3",
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::platform",
		.isSolid = false,
		.isPlatform = true,
		.isSupportV = true,
		.isSupportH = true,
		.onTileUpdate = updatePlatform,
		.onSpawn = spawnPlatform,
		.traits = traits,
	});
	mod.registerTile({
		.name = "platform::anchored::stub::right",
		.image = "core::tiles/platform@5",
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::platform",
		.isSolid = false,
		.isPlatform = true,
		.isSupportV = true,
		.isSupportH = true,
		.onTileUpdate = updatePlatform,
		.onSpawn = spawnPlatform,
		.traits = traits,
	});
	mod.registerTile({
		.name = "platform",
		.image = "core::tiles/platform@4",
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::platform",
		.isSolid = false,
		.isPlatform = true,
		.isSupportV = true,
		.isSupportH = true,
		.onTileUpdate = updatePlatform,
		.onSpawn = spawnPlatform,
		.traits = traits,
	});

	mod.registerTile({
		.name = "platform::anchored::left",
		.image = "core::tiles/platform@6",
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::platform",
		.isSolid = false,
		.isPlatform = true,
		.isSupportV = true,
		.isSupportH = true,
		.onTileUpdate = updatePlatform,
		.onSpawn = spawnPlatform,
		.traits = traits,
	});
	mod.registerTile({
		.name = "platform::anchored::right",
		.image = "core::tiles/platform@8",
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::platform",
		.isSolid = false,
		.isPlatform = true,
		.isSupportV = true,
		.isSupportH = true,
		.onTileUpdate = updatePlatform,
		.onSpawn = spawnPlatform,
		.traits = traits,
	});
	mod.registerTile({
		.name = "platform::supported::lone",
		.image = "core::tiles/platform@7",
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::platform",
		.isSolid = false,
		.isPlatform = true,
		.isSupportV = true,
		.isSupportH = true,
		.onTileUpdate = updatePlatform,
		.onSpawn = spawnPlatform,
		.traits = traits,
	});

	mod.registerTile({
		.name = "platform::supported::left",
		.image = "core::tiles/platform@9",
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::platform",
		.isSolid = false,
		.isPlatform = true,
		.isSupportV = true,
		.isSupportH = true,
		.onTileUpdate = updatePlatform,
		.onSpawn = spawnPlatform,
		.traits = traits,
	});
	mod.registerTile({
		.name = "platform::supported::right",
		.image = "core::tiles/platform@11",
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::platform",
		.isSolid = false,
		.isPlatform = true,
		.isSupportV = true,
		.isSupportH = true,
		.onTileUpdate = updatePlatform,
		.onSpawn = spawnPlatform,
		.traits = traits,
	});
	mod.registerTile({
		.name = "platform::supported::center",
		.image = "core::tiles/platform@10",
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::platform",
		.isSolid = false,
		.isPlatform = true,
		.isSupportV = true,
		.isSupportH = true,
		.onTileUpdate = updatePlatform,
		.onSpawn = spawnPlatform,
		.traits = traits,
	});
}

}
