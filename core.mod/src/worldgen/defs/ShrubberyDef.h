#pragma once

#include <stdint.h>
#include <swan/swan.h>
#include <PerlinNoise.hpp>

#include "../StructureDef.h"

namespace CoreMod {

class ShrubberyDef final: public StructureDef {
public:
	ShrubberyDef(Swan::World &world, uint32_t seed):
		tGrass_(world.getTileID("core::grass")),
		tDeadShrub_(world.getTileID("core::dead-shrub")),
		tBoulder_(world.getTileID("core::boulder")),
		tPotatoBush_(world.getTileID("core::potato-bush")),
		seed_(seed)
	{}

	void generateArea(Area &area) override;

private:
	Swan::Tile::ID tGrass_;
	Swan::Tile::ID tDeadShrub_;
	Swan::Tile::ID tBoulder_;
	Swan::Tile::ID tPotatoBush_;

	uint32_t seed_;
	siv::PerlinNoise perlin_{seed_};
};

}
