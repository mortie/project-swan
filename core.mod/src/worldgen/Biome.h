#pragma once

#include <swan/swan.h>
#include "PerlinNoise.hpp"
#include "worldgen/WGContext.h"
#include "worldgen/WorldArea.h"
#include "tiles.h"

namespace CoreMod {

template<float Min, float Max>
struct BiomeRange {
	constexpr BiomeRange(): normalizedValue(0) {}
	constexpr BiomeRange(float val): normalizedValue(normalize(val)) {}

	static constexpr BiomeRange fromNormalized(float norm)
	{
		BiomeRange r;
		r.normalizedValue = norm;
		return r;
	}

	// Value typically between -1 and 1
	float normalizedValue;

	static constexpr float normalize(float val) {
		return ((val - Min) / (Max - Min)) * 2 - 1;
	}

	constexpr float denorm() {
		return ((normalizedValue + 1) / 2) * (Max - Min) + Min;
	}
};

struct Biome {
	const char *name;

	// Relative humidity, percentage
	using Humidity = BiomeRange<0.f, 100.f>;
	Humidity humidity;

	// Degrees celsius
	using Temperature = BiomeRange<-20.f, 60.f>;
	Temperature temperature;

	// Meters above sea level
	using Elevation = BiomeRange<-50.f, 400.f>;
	Elevation elevation;

	// Sleepness of the terrain slope
	using Steepness = BiomeRange<0.f, 1.f>;
	Steepness steepness;

	Swan::Tile::ID &surfaceTile = tiles::grass;
	Swan::Tile::ID &soilTile = tiles::dirt;

	void (*postProcess)(WorldArea &area, WGContext &wg) = nullptr;
};

}
