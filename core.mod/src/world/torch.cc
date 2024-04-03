#include "torch.h"
#include "util.h"

namespace CoreMod {

static void onTorchSpawn(const Swan::Context &ctx, Swan::TilePos pos)
{
	// The default torch stands on the tile below it,
	// so if that's valid, we're good
	if (ctx.plane.getTile(pos.add(0, 1)).isOpaque) {
		return;
	}

	if (ctx.plane.getTile(pos.add(-1, 0)).isOpaque) {
		ctx.plane.setTile(pos, "core::torch::left");
	}
	else if (ctx.plane.getTile(pos.add(1, 0)).isOpaque) {
		ctx.plane.setTile(pos, "core::torch::right");
	}
	else {
		breakTileAndDropItem(ctx, pos);
	}
}

static void onTorchUpdate(const Swan::Context &ctx, Swan::TilePos pos)
{
	auto &tile = ctx.plane.getTile(pos);
	Swan::TilePos connection;

	if (tile.name == "core::torch") {
		connection = pos.add(0, 1);
	}
	else if (tile.name == "core::torch::left") {
		connection = pos.add(-1, 0);
	}
	else if (tile.name == "core::torch::right") {
		connection = pos.add(1, 0);
	}
	else {
		Swan::warn << "Torch update for unknown torch tile " << tile.name;
		return;
	}

	if (!ctx.plane.getTile(connection).isOpaque) {
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
		.droppedItem = "core::torch",
		.onSpawn = onTorchSpawn,
		.onTileUpdate = onTorchUpdate,
	});

	mod.registerTile({
		.name = "torch::left",
		.image = "core::tiles/torch/wall::left",
		.isSolid = false,
		.lightLevel = 80 / 255.0,
		.droppedItem = "core::torch",
		.onTileUpdate = onTorchUpdate,
	});

	mod.registerTile({
		.name = "torch::right",
		.image = "core::tiles/torch/wall::right",
		.isSolid = false,
		.lightLevel = 80 / 255.0,
		.droppedItem = "core::torch",
		.onTileUpdate = onTorchUpdate,
	});
}

}
