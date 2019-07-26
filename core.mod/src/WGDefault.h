#pragma once

#include <swan/swan.h>

#include <PerlinNoise/PerlinNoise.hpp>

class WGDefault: public Swan::WorldGen {
public:
	class Factory: public Swan::WorldGen::Factory {
	public:
		WorldGen *create(Swan::TileMap &tmap) { return new WGDefault(tmap); }
	};

	WGDefault(Swan::TileMap &tmap):
		tGrass_(tmap.getID("core::grass")), tDirt_(tmap.getID("core::dirt")),
		tStone_(tmap.getID("core::stone")), tAir_(tmap.getID("core::air")) {}

	void genChunk(Swan::WorldPlane &plane, Swan::Chunk &chunk) override;
	Swan::Entity &spawnPlayer(Swan::WorldPlane &plane) override;

private:
	Swan::Tile::ID tGrass_, tDirt_, tStone_, tAir_;
	siv::PerlinNoise perlin_ = siv::PerlinNoise(100);
};
