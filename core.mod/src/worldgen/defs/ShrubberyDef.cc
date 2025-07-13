#include "ShrubberyDef.h"

namespace CoreMod {

void ShrubberyDef::generateArea(Area &area)
{
	if (!area.hasSurface) {
		return;
	}

	auto shouldSpawn = [&](int x) {
		return Swan::random(seed_ + x) % 8 == 0;
	};

	for (int x = area.begin.x; x < area.end.x; ++x) {
		if (!shouldSpawn(x)) {
			continue;
		}

		int surfaceLevel = area.surfaceLevel(x);
		Swan::Tile::ID tile = area({x, surfaceLevel - 1});
		Swan::Tile::ID tileBelow = area({x, surfaceLevel});
		if (tileBelow == tGrass_ && tile == Swan::World::AIR_TILE_ID) {
			int r = Swan::random(seed_ * 3 + x) % 16;
			if (r > 14) {
				area({x, surfaceLevel - 1}) = tPotatoBush_;
			} else if (r > 9) {
				area({x, surfaceLevel - 1}) = tDeadShrub_;
			} else {
				area({x, surfaceLevel - 1}) = tBoulder_;
			}
		}
	}
}

}
