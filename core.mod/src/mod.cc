#include <swan/Mod.h>
#include <swan/Game.h>

extern "C" void mod_init(Swan::Mod &mod) {
	mod.init("core");

	mod.registerTile("air", "assets/tiles/air.png", Swan::Tile::Opts()
			.transparent());
	mod.registerTile("stone", "assets/tiles/stone.png", Swan::Tile::Opts());
	mod.registerTile("dirt", "assets/tiles/dirt.png", Swan::Tile::Opts());
	mod.registerTile("grass", "assets/tiles/grass.png", Swan::Tile::Opts());
}

int main() {
	Swan::Mod mod;
	mod.init("core");
}
