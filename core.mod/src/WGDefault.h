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
		tGrass_(world.getTileID("core::tree-trunk")), tDirt_(world.getTileID("core::dirt")),
		tStone_(world.getTileID("core::stone")), tAir_(world.getTileID("core::air")) {}

	void genChunk(Swan::WorldPlane &plane, Swan::Chunk &chunk) override;
	Swan::BodyTrait::HasBody *spawnPlayer(Swan::WorldPlane &plane) override;

private:
	Swan::Tile::ID tGrass_, tDirt_, tStone_, tAir_;
	siv::PerlinNoise perlin_ = siv::PerlinNoise(100);
};
