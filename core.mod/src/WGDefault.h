#pragma once

#include <swan/swan.h>

class WGDefault: public Swan::WorldGen {
public:
	class Factory: public Swan::WorldGen::Factory {
	public:
		WorldGen *create(Swan::TileMap &tmap) { return new WGDefault(tmap); }
	};

	Swan::Tile::ID tGrass_, tAir_;

	WGDefault(Swan::TileMap &tmap):
		tGrass_(tmap.getID("core::grass")), tAir_(tmap.getID("core::air")) {}

	void genChunk(Swan::WorldPlane &plane, Swan::Chunk &chunk, int x, int y);
};
