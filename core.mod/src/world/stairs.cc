#include "stairs.h"

namespace CoreMod {

static bool spawnStairs(Swan::Ctx &ctx, Swan::TilePos pos)
{
	if (!ctx.plane.tiles().get(pos.add(0, 1)).isSupportV()) {
		return false;
	}

	std::string_view variant;
	if (pos.x + 0.5 < ctx.world.player_->center().x) {
		variant = "core::stairs::left";
	} else {
		variant = "core::stairs::right";
	}

	ctx.plane.tiles().set(pos, variant);
	return true;
}

void registerStairs(Swan::Mod &mod)
{
	mod.registerTile({
		.name = "stairs",
		.image = "core::tiles/stairs::left",
		.isOpaque = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::stairs",
		.onSpawn = spawnStairs,
	});

	mod.registerTile({
		.name = "stairs::left",
		.image = "core::tiles/stairs::left",
		.isOpaque = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::stairs",
		.fluidCollision = std::make_shared<Swan::FluidCollision>(0b1111'0111'0011'0001),
	});

	mod.registerTile({
		.name = "stairs::right",
		.image = "core::tiles/stairs::right",
		.isOpaque = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::stairs",
		.fluidCollision = std::make_shared<Swan::FluidCollision>(0b1111'1110'1100'1000),
	});
}

}
