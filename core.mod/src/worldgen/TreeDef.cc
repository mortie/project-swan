#include "TreeDef.h"

namespace CoreMod {

void TreeDef::spawnTree(
	Swan::TilePos pos, Area &area)
{
	int height = 4 + Swan::random(pos.x) % 3;

	for (int y = pos.y; y >= pos.y - height; --y) {
		area({pos.x, y}) = tTreeTrunk_;
	}

	int radius = 2 + Swan::random(pos.x) % 2;
	int radius2 = radius * radius;

	Swan::TilePos top = pos - Swan::Vec2i{0, height};
	for (int ry = -radius; ry <= radius + 2; ++ry) {
		for (int rx = -radius; rx <= radius; ++rx) {
			if (rx == 0 && ry <= -radius / 2) {
				continue;
			}

			int d2 = std::max(
				ry * ry + rx * rx,
				(ry + 1) * (ry + 1) + rx * rx);
			if (d2 <= radius2) {
				area(top - Swan::Vec2i{rx, ry}) = tTreeLeaves_;
			}
		}
	}
}

void TreeDef::generateArea(Area &area)
{
	auto shouldSpawnTree = [&](int x) {
		return Swan::random(x ^ seed_) % 4 == 0;
	};

	if (area.end.y > 100 || area.begin.y < -100) {
		return;
	}

	for (int x = area.begin.x; x < area.end.x; ++x) {
		if (!shouldSpawnTree(x)) {
			continue;
		}

		// Avoid trees which are too close
		bool tooClose = false;
		for (int rx = 1; rx <= 5; ++rx) {
			if (shouldSpawnTree(x + rx)) {
				tooClose = true;
				break;
			}
		}

		if (tooClose) {
			continue;
		}

		for (int y = area.begin.y; y < area.end.y; ++y) {
			Swan::Tile::ID tile = area({x, y});
			Swan::Tile::ID tileBelow = area({x, y + 1});
			if (tileBelow == tGrass_ && tile == Swan::World::AIR_TILE_ID) {
				spawnTree({x, y}, area);
			}
		}
	}
}

}
