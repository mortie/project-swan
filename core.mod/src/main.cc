#include <swan/swan.h>

#include "DefaultWorldGen.h"
#include "entities/PlayerEntity.h"
#include "entities/ItemStackEntity.h"

static Swan::EventListener break_listener;

extern "C" void mod_init(Swan::Mod &mod, Swan::World &world) {
	mod.init("core");

	break_listener = world.evt_tile_break_.subscribe([](
			const Swan::Context &ctx, Swan::TilePos pos, Swan::Tile &tile) {

		if (tile.dropped_item_) {
			ctx.plane.spawnEntity(std::make_unique<ItemStackEntity>(
				ctx, pos, *tile.dropped_item_));
		}
	});

	mod.registerImage("tile/air");
	mod.registerImage("tile/stone");
	mod.registerImage("tile/dirt");
	mod.registerImage("tile/grass");
	mod.registerImage("tile/tree-trunk");
	mod.registerImage("tile/leaves");
	mod.registerImage("entity/player-running");
	mod.registerImage("entity/player-still");
	mod.registerImage("misc/background-cave");

	mod.registerTile({
		.name = "air",
		.image = "core/tile/air",
		.is_solid = false,
	});
	mod.registerTile({
		.name = "stone",
		.image = "core/tile/stone",
		.dropped_item = "core::stone",
	});
	mod.registerTile({
		.name = "dirt",
		.image = "core/tile/dirt",
		.dropped_item = "core::dirt",
	});
	mod.registerTile({
		.name = "grass",
		.image = "core/tile/grass",
		.dropped_item = "core::dirt",
	});
	mod.registerTile({
		.name = "tree-trunk",
		.image = "core/tile/tree-trunk",
		.dropped_item = "core::tree-trunk",
	});
	mod.registerTile({
		.name = "leaves",
		.image = "core/tile/leaves",
	});

	mod.registerItem({
		.name = "stone",
		.image = "core/tile/stone",
	});
	mod.registerItem({
		.name = "dirt",
		.image = "core/tile/dirt",
	});
	mod.registerItem({
		.name = "grass",
		.image = "core/tile/grass",
	});
	mod.registerItem({
		.name = "tree-trunk",
		.image = "core/tile/tree-trunk",
	});

	mod.registerWorldGen<DefaultWorldGen>("default");

	mod.registerEntity<PlayerEntity>("player");
	mod.registerEntity<ItemStackEntity>("item-stack");
}
