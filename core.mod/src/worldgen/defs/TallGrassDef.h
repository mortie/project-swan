#pragma once

#include <stdint.h>
#include <swan/swan.h>
#include <PerlinNoise.hpp>

#include "../StructureDef.h"

namespace CoreMod {

class TallGrassDef final: public StructureDef {
public:
	TallGrassDef(Swan::World &world, uint32_t seed):
		seed_(seed)
	{}

	void generateArea(WorldArea &area) override;

private:
	uint32_t seed_;
	siv::PerlinNoise perlin_{seed_};
};

}
