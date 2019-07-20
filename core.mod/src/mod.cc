#include <swan/Mod.h>
#include <swan/Game.h>

extern "C" void mod_init(Swan::Mod &mod) {
	mod.init("core");

	mod.registerTile("air", "assets/tiles/air.png");
	mod.registerTile("stone", "assets/tiles/stone.png");
	mod.registerTile("dirt", "assets/tiles/dirt.png");
	mod.registerTile("grass", "assets/tiles/grass.png");
}

int main() {
	Swan::Mod mod;
	mod.init("core");
}
