#include "chest.h"

#include "tileentities/ChestTileEntity.h"
#include "entities/PlayerEntity.h"

namespace CoreMod {

static void closeCallback(Swan::Ctx &ctx, Swan::EntityRef ref)
{
	auto pos = ref->trait<Swan::TileEntityTrait>()->pos;
	ctx.game.playSound(ctx.world.getSound("core::misc/lock-close"), pos);
	ctx.plane.tiles().set(pos, "core::chest");
}

static void openChest(Swan::Ctx &ctx, Swan::TilePos pos, Swan::Tile::ActivateMeta meta)
{
	auto *player = dynamic_cast<PlayerEntity *>(meta.activator.get());
	if (!player) {
		return;
	}

	auto self = ctx.plane.entities().getTileEntity(pos);
	if (!self) {
		return;
	}

	if (!player->askToOpenInventory(self, closeCallback)) {
		return;
	}

	ctx.game.playSound(ctx.world.getSound("core::misc/lock-open"), pos);
	ctx.plane.tiles().set(pos, "core::chest::open");
}

static void closeChest(Swan::Ctx &ctx, Swan::TilePos pos, Swan::Tile::ActivateMeta meta)
{
	auto *player = dynamic_cast<PlayerEntity *>(meta.activator.get());
	if (!player) {
		return;
	}

	auto self = ctx.plane.entities().getTileEntity(pos);
	if (!self) {
		return;
	}

	player->askToCloseInventory(ctx, self);
}

void registerChest(Swan::Mod &mod)
{
	mod.registerEntity<ChestTileEntity>("tile::chest");

	mod.registerTile({
		.name = "chest",
		.image = "core::tiles/chest::closed",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::chest",
		.tileEntity = "core::tile::chest",
		.onActivate = openChest,
	});
	mod.registerTile({
		.name = "chest::open",
		.image = "core::tiles/chest::open",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::chest",
		.tileEntity = "core::tile::chest",
		.onActivate = closeChest,
	});
}

}
