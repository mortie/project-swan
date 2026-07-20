#pragma once

#include "worldgen/WGContext.h"
#include "worldgen/WorldArea.h"
#include "tiles.h"
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

struct OutcropDef {
	Swan::Tile::ID tile;
	int probability;
};

inline void generateOutcrops(
	WorldArea &area, WGContext &wg, int frequency,
	Swan::Vec2i rangeY, std::initializer_list<OutcropDef> defs)
{
	// We expect all outcrop defs to have a tile which represents the first of 4:
	// a normal upright variant, a hanging variant, a left variant, and a right variant.
	constexpr int NORMAL = 0;
	constexpr int HANGING = 1;
	constexpr int LEFT = 2;
	constexpr int RIGHT = 3;

	int maxProb = 0;
	for (auto &def: defs) {
		maxProb += def.probability;
	}

	for (int y = area.begin.y; y <= area.end.y; ++y) {
		for (int x = area.begin.x; x < area.end.x; ++x) {
			int surface = area.surfaceLevel(x);

			if (y < rangeY.x + surface || y > rangeY.y + surface) {
				continue;
			}

			if (Swan::random((x + wg.seed) ^ (y * 33)) % frequency != 0) {
				continue;
			}

			auto &id = area({x, y});
			if (id != Swan::World::AIR_TILE_ID) {
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

			if (area({x, y + 1}) == tiles::stone) {
				id = it->tile + NORMAL;
			} else if (area({x, y - 1}) == tiles::stone) {
				id = it->tile + HANGING;
			} else if (area({x - 1, y}) == tiles::stone) {
				id = it->tile + LEFT;
			} else if (area({x + 1, y}) == tiles::stone) {
				id = it->tile + RIGHT;
			}
		}
	}
}

}
