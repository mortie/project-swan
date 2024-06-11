#include "item-fan.h"

#include "tileentities/ItemFanTileEntity.h"
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

	Swan::Direction dir = Swan::Direction::LEFT;
	if (hasRight) {
		ctx.plane.setTile(pos, "core::item-fan::right");
		dir = Swan::Direction::RIGHT;
	} else {
		ctx.plane.setTile(pos, "core::item-fan::left");
		dir = Swan::Direction::LEFT;
	}

	Swan::Entity *ent = ctx.plane.getTileEntity(pos).get();
	if (ent) {
		dynamic_cast<ItemFanTileEntity *>(ent)->setDirection(dir);
	}
}

void registerItemFan(Swan::Mod &mod)
{
	mod.registerEntity<ItemFanTileEntity>("tile::item-fan");

	mod.registerTile({
		.name = "item-fan",
		.image = "core::tiles/item-fan",
		.isSolid = true,
		.droppedItem = "core::item-fan",
		.onTileUpdate = updateItemFan,
		.traits = std::make_shared<PipeConnectibleTileTrait>(),
	});

	mod.registerTile({
		.name = "item-fan::left",
		.image = "core::tiles/item-fan::left",
		.isSolid = true,
		.droppedItem = "core::item-fan",
		.onTileUpdate = updateItemFan,
		.tileEntity = "core::tile::item-fan",
		.traits = std::make_shared<PipeConnectibleTileTrait>(
			Swan::Direction::LEFT),
	});

	mod.registerTile({
		.name = "item-fan::right",
		.image = "core::tiles/item-fan::right",
		.isSolid = true,
		.droppedItem = "core::item-fan",
		.onTileUpdate = updateItemFan,
		.tileEntity = "core::tile::item-fan",
		.traits = std::make_shared<PipeConnectibleTileTrait>(
			Swan::Direction::RIGHT),
	});
}

}
