#include <swan/swan.h>

#include "WGDefault.h"
#include "entities/EntPlayer.h"
#include "entities/EntItemStack.h"

extern "C" void mod_init(Swan::Mod &mod) {
	mod.init("core");

	mod.registerTile("air", new Swan::Tile{
		.image = mod.loadImage("assets/tiles/air.png"),
		.is_solid = false,
	});
	mod.registerTile("stone", new Swan::Tile{
		.image = mod.loadImage("assets/tiles/stone.png"),
		.dropped_item = "core::stone",
	});
	mod.registerTile("dirt", new Swan::Tile{
		.image = mod.loadImage("assets/tiles/dirt.png"),
		.dropped_item = "core::dirt",
	});
	mod.registerTile("grass", new Swan::Tile{
		.image = mod.loadImage("assets/tiles/grass.png"),
		.dropped_item = "core::grass",
	});

	mod.registerItem("stone", new Swan::Item{
		.image = mod.loadImage("assets/tiles/stone.png"),
	});
	mod.registerItem("dirt", new Swan::Item{
		.image = mod.loadImage("assets/tiles/stone.png"),
	});
	mod.registerItem("grass", new Swan::Item{
		.image = mod.loadImage("assets/tiles/grass.png"),
	});

	mod.registerWorldGen("default", new WGDefault::Factory());

	mod.registerEntity("player", new EntPlayer::Factory());
	mod.registerEntity("item-stack", new EntItemStack::Factory());

	mod.registerAsset("player-running", new Swan::Asset("assets/entities/player-running.png"));
	mod.registerAsset("player-still", new Swan::Asset("assets/entities/player-still.png"));
}
