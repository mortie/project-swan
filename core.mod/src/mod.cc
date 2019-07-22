#include <swan/Mod.h>

class DefaultWorldGen: public Swan::WorldGen {
public:
	class Factory: public Swan::WorldGen::Factory {
	public:
		Factory(): Swan::WorldGen::Factory("Default") {}
		WorldGen *create(Swan::TileMap &tmap) { return new DefaultWorldGen(tmap); }
	};

	Swan::Tile::ID tGrass_, tAir_;

	DefaultWorldGen(Swan::TileMap &tmap):
		tGrass_(tmap.getID("core::grass")), tAir_(tmap.getID("core::air")) {}

	void genChunk(Swan::Chunk &chunk, int x, int y) {
		fprintf(stderr, "genChunk at %i, %i\n", x, y);
		for (int x = 0; x < Swan::CHUNK_WIDTH; ++x) {
			for (int y = 0; y < Swan::CHUNK_HEIGHT; ++y) {
				chunk.tiles_[x][y] = y == 3 ? tGrass_ : tAir_;
			}
		}
	}
};

extern "C" void mod_init(Swan::Mod &mod) {
	mod.init("core");

	mod.registerTile("air", "assets/tiles/air.png", Swan::Tile::Opts()
			.solid(false));
	mod.registerTile("stone", "assets/tiles/stone.png", Swan::Tile::Opts());
	mod.registerTile("dirt", "assets/tiles/dirt.png", Swan::Tile::Opts());
	mod.registerTile("grass", "assets/tiles/grass.png", Swan::Tile::Opts());

	mod.registerWorldGen(new DefaultWorldGen::Factory());
}

int main() {
	Swan::Mod mod;
	mod.init("core");
}
