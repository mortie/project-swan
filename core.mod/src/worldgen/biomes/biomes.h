#pragma once

#include "worldgen/Biome.h"
#include <array>

namespace CoreMod::biomes {

extern Biome desert;
extern Biome grassland;
extern Biome snowland;

inline std::array biomes = {
	&desert,
	&grassland,
	&snowland,
};

}
