#pragma once

#include "worldgen/Prefab.h"
#include <swan/swan.h>
#include <cstdint>
#include <PerlinNoise.hpp>
#include <vector>

namespace CoreMod {

struct WGContext {
	WGContext(uint32_t seed, Swan::World &world):
		seed(seed),
		perlin(seed),
		world(world) {}

	uint32_t seed;
	siv::PerlinNoise perlin;
	Swan::World &world;
};

}
