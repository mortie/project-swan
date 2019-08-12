#include <swan/swan.h>

#include "WGDefault.h"
#include "entities/EntPlayer.h"

extern "C" void mod_init(Swan::Mod &mod) {
	mod.init("core");

	mod.registerTile("air", new Swan::Tile{
		.image = mod.loadImage("assets/tiles/air.png"),
		.is_solid = false,
		.dropped_item = "dirt",
	});
	mod.registerTile("stone", new Swan::Tile{
		.image = mod.loadImage("assets/tiles/stone.png"),
		.dropped_item = "stone",
	});
	mod.registerTile("dirt", new Swan::Tile{
		.image = mod.loadImage("assets/tiles/dirt.png"),
		.dropped_item = "dirt",
	});
	mod.registerTile("grass", new Swan::Tile{
		.image = mod.loadImage("assets/tiles/grass.png"),
		.dropped_item = "grass",
	});

	mod.registerWorldGen("default", new WGDefault::Factory());

	mod.registerEntity("player", new EntPlayer::Factory());

	mod.registerAsset("player-running", new Swan::Asset("assets/entities/player-running.png"));
	mod.registerAsset("player-still", new Swan::Asset("assets/entities/player-still.png"));
}

int main() {
	Swan::Mod mod;
	mod.init("core");
}
