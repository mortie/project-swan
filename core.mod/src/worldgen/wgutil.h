#pragma once

#include "worldgen/WGContext.h"
#include "worldgen/WorldArea.h"
#include <swan/swan.h>

namespace CoreMod {

struct SurfaceStructureDef {
	Swan::Tile::ID below;
	Swan::Tile::ID tile;
	int probability;
};

inline void generateSurfaceShrubs(
	WorldArea &area, WGContext &wg, int frequency,
	std::initializer_list<SurfaceStructureDef> defs)
{
	if (!area.hasSurface) {
		return;
	}

	int maxProb = 0;
	for (auto &def: defs) {
		maxProb += def.probability;
	}

	auto shouldSpawnShrub = [&](int x) {
		return Swan::random(wg.seed + x) % frequency == 0;
	};

	for (int x = area.begin.x; x < area.end.x; ++x) {
		if (!shouldSpawnShrub(x)) {
			continue;
		}

		int r = Swan::random(wg.seed * 3 + x) % maxProb;
		int c = 0;
		auto it = defs.begin();
		for (int i = 0; i < r; ++i) {
			c += 1;
			if (c >= it->probability) {
				it++;
				c = 0;
			}
		}

		int surfaceLevel = area.surfaceLevel(x);
		Swan::Tile::ID tileBelow = area({x, surfaceLevel});
		if (tileBelow != it->below) {
			continue;
		}

		Swan::Tile::ID tile = area({x, surfaceLevel - 1});
		if (tile != Swan::World::AIR_TILE_ID) {
			continue;
		}

		area({x, surfaceLevel - 1}) = it->tile;
	}
}

}
