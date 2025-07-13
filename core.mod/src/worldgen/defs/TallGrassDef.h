#pragma once

#include <stdint.h>
#include <swan/swan.h>
#include <PerlinNoise.hpp>

#include "../StructureDef.h"

namespace CoreMod {

class TallGrassDef final: public StructureDef {
public:
	TallGrassDef(Swan::World &world, uint32_t seed):
		tGrass_(world.getTileID("core::grass")),
		tTallGrass_(world.getTileID("core::tall-grass")),
		seed_(seed)
	{}

	void generateArea(Area &area) override;

private:
	Swan::Tile::ID tGrass_;
	Swan::Tile::ID tTallGrass_;

	uint32_t seed_;
	siv::PerlinNoise perlin_{seed_};
};

}
