#pragma once

#include <swan/swan.h>

#include <PerlinNoise/PerlinNoise.hpp>

class WGDefault: public Swan::WorldGen {
public:
	class Factory: public Swan::WorldGen::Factory {
	public:
		WorldGen *create(Swan::World &world) override { return new WGDefault(world); }
	};

	WGDefault(Swan::World &world):
		tGrass_(world.getTileID("core::grass")), tDirt_(world.getTileID("core::dirt")),
		tStone_(world.getTileID("core::stone")), tAir_(world.getTileID("core::air")),
		tTreeTrunk_(world.getTileID("core::tree-trunk")), tLeaves_(world.getTileID("core::leaves")) {}

	void genChunk(Swan::WorldPlane &plane, Swan::Chunk &chunk) override;
	Swan::BodyTrait::HasBody *spawnPlayer(Swan::WorldPlane &plane) override;

private:
	Swan::Tile::ID genTile(Swan::TilePos pos);
	Swan::Tile::ID tGrass_, tDirt_, tStone_, tAir_, tTreeTrunk_, tLeaves_;
	siv::PerlinNoise perlin_ = siv::PerlinNoise(100);
};
