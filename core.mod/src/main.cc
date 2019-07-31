#include <swan/swan.h>

#include "WGDefault.h"
#include "entities/EntPlayer.h"

extern "C" void mod_init(Swan::Mod &mod) {
	mod.init("core");

	mod.registerTile("air", (new Swan::Tile("assets/tiles/air.png"))->solid(false));
	mod.registerTile("stone", (new Swan::Tile("assets/tiles/stone.png")));
	mod.registerTile("dirt", (new Swan::Tile("assets/tiles/dirt.png")));
	mod.registerTile("grass", (new Swan::Tile("assets/tiles/grass.png")));

	mod.registerWorldGen("default", new WGDefault::Factory());

	mod.registerEntity("player", new EntPlayer::Factory());

	mod.registerAsset("player-running", new Swan::Asset("assets/entities/player-running.png"));
	mod.registerAsset("player-still", new Swan::Asset("assets/entities/player-still.png"));
}

int main() {
	Swan::Mod mod;
	mod.init("core");
}
