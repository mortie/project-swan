#include <swan/Mod.h>
#include <swan/Game.h>

extern "C" void mod_init(Swan::Mod &mod) {
	mod.init("core");

	mod.registerTile("test1", Swan::Tile());
	mod.registerTile("test2", Swan::Tile());
}

int main() {
	Swan::Mod mod;
	mod.init("core");
}
