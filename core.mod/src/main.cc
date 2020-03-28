#include <swan/swan.h>

#include "DefaultWorldGen.h"
#include "entities/EntPlayer.h"
#include "entities/EntItemStack.h"

extern "C" void mod_init(Swan::Mod &mod) {
	mod.init("core");

	mod.registerImage({ "air", "tiles/air.png" });
	mod.registerImage({ "stone", "tiles/stone.png" });
	mod.registerImage({ "dirt", "tiles/dirt.png" });
	mod.registerImage({ "grass", "tiles/grass.png" });
	mod.registerImage({ "tree-trunk", "tiles/tree-trunk.png" });
	mod.registerImage({ "leaves", "tiles/leaves.png" });
	mod.registerImage({ "player-running", "entities/player-running.png", 64 });
	mod.registerImage({ "player-still", "entities/player-still.png", 64 });
	mod.registerImage({ "background-cave", "misc/background-cave.png" });
	mod.registerImage({ "background-cave-2", "misc/background-cave-2.png" });

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
	mod.registerTile({
		.name = "tree-trunk",
		.image = "core::tree-trunk",
		.dropped_item = "core::tree-trunk",
	});
	mod.registerTile({
		.name = "leaves",
		.image = "core::leaves",
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
	mod.registerItem({
		.name = "tree-trunk",
		.image = "core::tree-trunk",
	});

	mod.registerWorldGen("default", std::make_unique<DefaultWorldGen::Factory>());

	mod.registerEntity("player", std::make_unique<EntPlayer::Factory>());
	mod.registerEntity("item-stack", std::make_unique<EntItemStack::Factory>());
}
