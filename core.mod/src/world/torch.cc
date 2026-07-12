#include "torch.h"
#include "util.h"
#include "tiles.h"

namespace CoreMod {

template<Swan::Tile::ID &Base>
static bool onTorchSpawn(Swan::Ctx &ctx, Swan::TilePos pos)
{
	// The default torch stands on the tile below it,
	// so if that's valid, we're good
	if (ctx.plane.tiles().get(pos.add(0, 1)).isSupportV()) {
		return true;
	}

	if (ctx.plane.tiles().get(pos.add(-1, 0)).isFullSupportH()) {
		ctx.plane.tiles().setID(pos, Base + 1);
		return true;
	}
	else if (ctx.plane.tiles().get(pos.add(1, 0)).isFullSupportH()) {
		ctx.plane.tiles().setID(pos, Base + 2);
		return true;
	}

	return false;
}

template<Swan::Tile::ID &Base>
static void onTorchUpdate(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto id = ctx.plane.tiles().getID(pos);
	bool isSupported;

	if (id == Base) {
		isSupported = ctx.plane.tiles().get(pos.add(0, 1)).isSupportV();
	}
	else if (id == Base + 1) {
		isSupported = ctx.plane.tiles().get(pos.add(-1, 0)).isFullSupportH();
	}
	else if (id == Base + 2) {
		isSupported = ctx.plane.tiles().get(pos.add(1, 0)).isFullSupportH();
	}
	else {
		Swan::warn << "Torch update for unknown torch tile " << ctx.world.getTileByID(id).name;
		return;
	}

	if (!isSupported) {
		breakTileAndDropItem(ctx, pos);
	}
}

void registerTorch(Swan::Mod &mod)
{
	float lightLevel = 80 / 255.0;
	float temperature = 0.7;

	mod.registerTile({
		.name = "torch",
		.image = "core::tiles/torch::normal",
		.isSolid = false,
		.lightLevel = lightLevel,
		.temperature = temperature,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::torch",
		.onSpawn = onTorchSpawn<tiles::torch>,
		.onTileUpdate = onTorchUpdate<tiles::torch>,
		.onWorldTick = breakIfInFluid,
	});

	mod.registerTile({
		.name = "torch::left",
		.image = "core::tiles/torch::left",
		.isSolid = false,
		.lightLevel = lightLevel,
		.temperature = temperature,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::torch",
		.onTileUpdate = onTorchUpdate<tiles::torch>,
		.onWorldTick = breakIfInFluid,
	});

	mod.registerTile({
		.name = "torch::right",
		.image = "core::tiles/torch::right",
		.isSolid = false,
		.lightLevel = lightLevel,
		.temperature = temperature,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::torch",
		.onTileUpdate = onTorchUpdate<tiles::torch>,
		.onWorldTick = breakIfInFluid,
	});
}

void registerScorchbloomTorch(Swan::Mod &mod)
{
	float lightLevel = 100 / 255.0;
	float temperature = 0.9;

	mod.registerTile({
		.name = "scorchbloom-torch",
		.image = "core::tiles/scorchbloom-torch::normal",
		.isSolid = false,
		.lightLevel = lightLevel,
		.temperature = temperature,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::scorchbloom-torch",
		.onSpawn = onTorchSpawn<tiles::scorchbloomTorch>,
		.onTileUpdate = onTorchUpdate<tiles::scorchbloomTorch>,
	});

	mod.registerTile({
		.name = "scorchbloom-torch::left",
		.image = "core::tiles/scorchbloom-torch::left",
		.isSolid = false,
		.lightLevel = lightLevel,
		.temperature = temperature,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::scorchbloom-torch",
		.onTileUpdate = onTorchUpdate<tiles::scorchbloomTorch>,
	});

	mod.registerTile({
		.name = "scorchbloom-torch::right",
		.image = "core::tiles/scorchbloom-torch::right",
		.isSolid = false,
		.lightLevel = lightLevel,
		.temperature = temperature,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::scorchbloom-torch",
		.onTileUpdate = onTorchUpdate<tiles::scorchbloomTorch>,
	});
}

}
