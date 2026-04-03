#pragma once

#include "worldgen/Biome.h"
#include <array>

namespace CoreMod::biomes {

	extern Biome desert;
extern Biome grassland;

inline std::array biomes = {
	&desert,
	&grassland,
};

}
