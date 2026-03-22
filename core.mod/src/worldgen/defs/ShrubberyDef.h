#pragma once

#include <stdint.h>
#include <swan/swan.h>
#include <PerlinNoise.hpp>

#include "../StructureDef.h"

namespace CoreMod {

class ShrubberyDef final: public StructureDef {
public:
	ShrubberyDef(Swan::World &world, uint32_t seed):
		seed_(seed)
	{}

	void generateArea(WorldArea &area) override;

private:
	uint32_t seed_;
	siv::PerlinNoise perlin_{seed_};
};

}
