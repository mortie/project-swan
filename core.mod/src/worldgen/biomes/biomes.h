#pragma once

#include "worldgen/Biome.h"
#include <array>

namespace CoreMod::biomes {

extern Biome desert;
extern Biome grasslands;
extern Biome snowlands;

inline std::array biomes = {
	&desert,
	&grasslands,
	&snowlands,
};

}
