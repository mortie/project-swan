#include "TreeDef.h"

namespace CoreMod {

static void spawnTree(
	Swan::World &world, Swan::TilePos pos,
	std::unordered_map<Swan::TilePos, Swan::Tile::ID> &map)
{
	Swan::Tile::ID logID = world.getTileID("core::tree-trunk");
	Swan::Tile::ID leavesID = world.getTileID("core::tree-leaves");
	int height = 4 + Swan::random(pos.x) % 3;

	for (int y = pos.y; y >= pos.y - height; --y) {
		map[{pos.x, y}] = logID;
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
				map[top - Swan::Vec2i{rx, ry}] = leavesID;
			}
		}
	}
}

void TreeDef::generateArea(
	const Meta &meta, Swan::TilePos pos, Swan::Vec2i size,
	std::unordered_map<Swan::TilePos, Swan::Tile::ID> &map)
{
	auto shouldSpawnTree = [&](int x) {
		return Swan::random(x ^ seed_) % 4 == 0;
	};

	for (int rx = 0; rx < size.x; ++rx) {
		int x = pos.x + rx;
		int grassLevel = meta.grassLevels[rx];
		if (!shouldSpawnTree(x)) {
			continue;
		}

		// Avoid trees which are too close
		bool close = false;
		for (int rx = 1; rx <= 5; ++rx) {
			if (shouldSpawnTree(x + rx)) {
				close = true;
				break;
			}
		}

		if (close) {
			continue;
		}

		spawnTree(meta.world, {x, grassLevel - 1}, map);
	}
}

}
