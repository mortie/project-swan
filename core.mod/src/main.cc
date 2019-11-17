#include <swan/swan.h>

#include "WGDefault.h"
#include "entities/EntPlayer.h"
#include "entities/EntItemStack.h"

extern "C" void mod_init(Swan::Mod &mod) {
	mod.init("core");

	mod.registerImage("air", "tiles/air.png");
	mod.registerImage("stone", "tiles/stone.png");
	mod.registerImage("dirt", "tiles/dirt.png");
	mod.registerImage("grass", "tiles/grass.png");
	mod.registerImage("player-running", "entities/player-running.png");
	mod.registerImage("player-still", "entities/player-still.png");

	mod.registerTile({
		.name = "air",
		.image = "core::air",
		.is_solid = false,
	});
	mod.registerTile({
		.name = "stone",
		.image = "core::stone",
		.dropped_item = "core::stone",
	});
	mod.registerTile({
		.name = "dirt",
		.image = "core::dirt",
		.dropped_item = "core::dirt",
	});
	mod.registerTile({
		.name = "grass",
		.image = "core::grass",
		.dropped_item = "core::dirt",
	});

	mod.registerItem({
		.name = "stone",
		.image = "core::stone",
	});
	mod.registerItem({
		.name = "dirt",
		.image = "core::dirt",
	});
	mod.registerItem({
		.name = "grass",
		.image = "core::grass",
	});

	mod.registerWorldGen("default", new WGDefault::Factory());

	mod.registerEntity("player", new EntPlayer::Factory());
	mod.registerEntity("item-stack", new EntItemStack::Factory());
}
