#include "item-fan.h"

#include "tileentities/ItemFanTileEntity.h"
#include "pipe.h"

namespace CoreMod {

static void updateItemFan(Swan::Ctx &ctx, Swan::TilePos pos)
{
	bool hasLeft = ctx.plane.entities().getTileEntity(pos + Swan::Direction::LEFT)
		->trait<Swan::InventoryTrait>();
	bool hasRight = ctx.plane.entities().getTileEntity(pos + Swan::Direction::RIGHT)
		->trait<Swan::InventoryTrait>();

	auto &tile = ctx.plane.tiles().get(pos);

	if (tile.name.str().ends_with("::left") && hasLeft) {
		return;
	}

	if (tile.name.str().ends_with("::right") && hasRight) {
		return;
	}

	Swan::Direction dir = Swan::Direction::LEFT;
	if (hasRight) {
		ctx.plane.tiles().set(pos, "core::item-fan::right");
		dir = Swan::Direction::RIGHT;
	} else {
		ctx.plane.tiles().set(pos, "core::item-fan::left");
		dir = Swan::Direction::LEFT;
	}

	Swan::Entity *ent = ctx.plane.entities().getTileEntity(pos).get();
	if (ent) {
		dynamic_cast<ItemFanTileEntity *>(ent)->setDirection(dir);
	}
}

void registerItemFan(Swan::Mod &mod)
{
	mod.registerEntity<ItemFanTileEntity>("tile::item-fan");

	mod.registerTile({
		.name = "item-fan",
		.image = "core::tiles/item-fan::left",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::item-fan",
		.onTileUpdate = updateItemFan,
		.traits = std::make_shared<PipeConnectibleTileTrait>(),
	});

	mod.registerTile({
		.name = "item-fan::left",
		.image = "core::tiles/item-fan::left",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::item-fan",
		.tileEntity = "core::tile::item-fan",
		.onTileUpdate = updateItemFan,
		.traits = std::make_shared<PipeConnectibleTileTrait>(
			Swan::Direction::LEFT),
	});

	mod.registerTile({
		.name = "item-fan::right",
		.image = "core::tiles/item-fan::right",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::item-fan",
		.tileEntity = "core::tile::item-fan",
		.onTileUpdate = updateItemFan,
		.traits = std::make_shared<PipeConnectibleTileTrait>(
			Swan::Direction::RIGHT),
	});
}

}
