#include "world/burner-generator.h"
#include "bonfire.h"

#include "entities/PlayerEntity.h"
#include "util.h"
#include "tileentities/BurnerGeneratorTileEntity.h"
#include "tiles.h"

namespace CoreMod {

static bool onActivate(Swan::Ctx &ctx, Swan::TilePos pos, Swan::Tile::ActivateMeta meta)
{
	auto *player = dynamic_cast<PlayerEntity *>(meta.activator.get());
	if (!player) {
		return false;
	}

	auto self = ctx.plane.entities().getTileEntity(pos);
	if (!self) {
		return false;
	}

	if (player->currentInventoryEntity() == self) {
		player->askToCloseInventory(ctx, self);
		return true;
	}

	if (!player->askToOpenInventory(self, nullptr)) {
		return false;
	}

	return true;
}

void registerBurnerGenerator(Swan::Mod &mod)
{
	mod.registerEntity<BurnerGeneratorTileEntity>("tile::burner-generator");

	mod.registerTile({
		.name = "burner-generator",
		.image = "core::tiles/burner-generator",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::burner-generator",
		.tileEntity = "core::tile::burner-generator",
		.onActivate = onActivate,
	});
}

}
