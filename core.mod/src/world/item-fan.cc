#include "item-fan.h"

#include <optional>

#include "pipe.h"

namespace CoreMod {

static void updateItemFan(const Swan::Context &ctx, Swan::TilePos pos)
{
	bool hasLeft = ctx.plane.getTileEntity(pos + Swan::Direction::LEFT)
		->trait<Swan::InventoryTrait>();
	bool hasRight = ctx.plane.getTileEntity(pos + Swan::Direction::RIGHT)
		->trait<Swan::InventoryTrait>();

	auto &tile = ctx.plane.getTile(pos);

	if (tile.name.ends_with("::left") && hasLeft) {
		return;
	}

	if (tile.name.ends_with("::right") && hasRight) {
		return;
	}

	if (hasLeft) {
		ctx.plane.setTile(pos, "core::item-fan::left");
	} else if (hasRight) {
		ctx.plane.setTile(pos, "core::item-fan::right");
	}
}

void registerItemFan(Swan::Mod &mod)
{
	mod.registerTile({
		.name = "item-fan",
		.image = "core::tiles/item-fan",
		.isSolid = true,
		.droppedItem = "core::item-fan",
		.onTileUpdate = updateItemFan,
		.traits = std::make_shared<PipeConnectibleTileTrait>(),
	});

	for (auto direction: {"left", "right"}) {
		mod.registerTile({
			.name = Swan::cat("item-fan::", direction),
			.image = Swan::cat("core::tiles/item-fan::", direction),
			.isSolid = true,
			.droppedItem = "core::item-fan",
			.onTileUpdate = updateItemFan,
			.traits = std::make_shared<PipeConnectibleTileTrait>(),
		});
	}
}

}
