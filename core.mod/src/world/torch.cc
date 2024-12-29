#include "torch.h"
#include "util.h"

namespace CoreMod {

static bool onTorchSpawn(const Swan::Context &ctx, Swan::TilePos pos)
{
	// The default torch stands on the tile below it,
	// so if that's valid, we're good
	if (ctx.plane.tiles().get(pos.add(0, 1)).isSupportV) {
		return true;
	}

	if (ctx.plane.tiles().get(pos.add(-1, 0)).isSupportH) {
		ctx.plane.tiles().set(pos, "core::torch::left");
		return true;
	}
	else if (ctx.plane.tiles().get(pos.add(1, 0)).isSupportH) {
		ctx.plane.tiles().set(pos, "core::torch::right");
		return true;
	}

	return false;
}

static void onTorchUpdate(const Swan::Context &ctx, Swan::TilePos pos)
{
	auto &tile = ctx.plane.tiles().get(pos);
	bool isSupported;

	if (tile.name == "core::torch") {
		isSupported = ctx.plane.tiles().get(pos.add(0, 1)).isSupportV;
	}
	else if (tile.name == "core::torch::left") {
		isSupported = ctx.plane.tiles().get(pos.add(-1, 0)).isSupportH;
	}
	else if (tile.name == "core::torch::right") {
		isSupported = ctx.plane.tiles().get(pos.add(1, 0)).isSupportH;
	}
	else {
		Swan::warn << "Torch update for unknown torch tile " << tile.name;
		return;
	}

	if (!isSupported) {
		breakTileAndDropItem(ctx, pos);
	}
}

void registerTorch(Swan::Mod &mod)
{
	mod.registerTile({
		.name = "torch",
		.image = "core::tiles/torch/normal",
		.isSolid = false,
		.lightLevel = 80 / 255.0,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::torch",
		.onSpawn = onTorchSpawn,
		.onTileUpdate = onTorchUpdate,
	});

	mod.registerTile({
		.name = "torch::left",
		.image = "core::tiles/torch/wall::left",
		.isSolid = false,
		.lightLevel = 80 / 255.0,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::torch",
		.onTileUpdate = onTorchUpdate,
	});

	mod.registerTile({
		.name = "torch::right",
		.image = "core::tiles/torch/wall::right",
		.isSolid = false,
		.lightLevel = 80 / 255.0,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::torch",
		.onTileUpdate = onTorchUpdate,
	});
}

}
