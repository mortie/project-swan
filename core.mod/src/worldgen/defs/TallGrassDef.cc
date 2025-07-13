#include "TallGrassDef.h"

namespace CoreMod {

void TallGrassDef::generateArea(Area &area)
{
	if (!area.hasSurface) {
		return;
	}

	auto shouldSpawnGrass = [&](int x) {
		return perlin_.noise2D(x / 20.6, 0) > 0.2;
	};

	for (int x = area.begin.x; x < area.end.x; ++x) {
		if (!shouldSpawnGrass(x)) {
			continue;
		}

		for (int y = area.begin.y; y < area.end.y; ++y) {
			Swan::Tile::ID tile = area({x, y});
			Swan::Tile::ID tileBelow = area({x, y + 1});
			if (tileBelow == tGrass_ && tile == Swan::World::AIR_TILE_ID) {
				area({x, y}) = tTallGrass_;
			}
		}
	}
}

}
