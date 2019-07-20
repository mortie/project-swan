#include <swan/Mod.h>
#include <swan/Game.h>

extern "C" void mod_init(Swan::Mod &mod) {
	mod.init("core");

	mod.registerTile("stone", Swan::Tile("assets/tiles/stone.png"));
	mod.registerTile("dirt", Swan::Tile("assets/tiles/dirt.png"));
	mod.registerTile("grass", Swan::Tile("assets/tiles/grass.png"));
}

int main() {
	Swan::Mod mod;
	mod.init("core");
}
