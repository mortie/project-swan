#pragma once

#include <swan/swan.h>

class WGDefault: public Swan::WorldGen {
public:
	class Factory: public Swan::WorldGen::Factory {
	public:
		WorldGen *create(Swan::TileMap &tmap) { return new WGDefault(tmap); }
	};

	Swan::Tile::ID tGrass_, tDirt_, tStone_, tAir_;

	WGDefault(Swan::TileMap &tmap):
		tGrass_(tmap.getID("core::grass")), tDirt_(tmap.getID("core::dirt")),
		tStone_(tmap.getID("core::stone")), tAir_(tmap.getID("core::air")) {}

	void genChunk(Swan::WorldPlane &plane, Swan::Chunk &chunk) override;
	Swan::Entity &spawnPlayer(Swan::WorldPlane &plane) override;
};
