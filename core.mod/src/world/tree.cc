#include "tree.h"
#include "world/util.h"

#include <algorithm>
#include <queue>
#include <set>
#include <utility>

namespace CoreMod {

void spawnTree(const Swan::Context &ctx, Swan::TilePos pos)
{
	Swan::Tile::ID logID = ctx.world.getTileID("core::tree-trunk");
	Swan::Tile::ID leavesID = ctx.world.getTileID("core::tree-leaves");
	int height = 4 + Swan::random(pos.x) % 3;

	for (int y = pos.y; y >= pos.y - height; --y) {
		ctx.plane.setTileIDWithoutUpdate({pos.x, y}, logID);
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
				ctx.plane.setTileIDWithoutUpdate(top - Swan::Vec2i{rx, ry}, leavesID);
			}
		}
	}
}

void breakTreeLeavesIfFloating(const Swan::Context &ctx, Swan::TilePos pos)
{
	/*
	 * This algorithm implements a kind of search algorithm,
	 * where connected leaves are searched until a trunk is found.
	 * If no trunk is found, the leaf breaks.
	 */

	struct SearchTile {
		Swan::TilePos pos;
		int dist;
	};

	bool foundTrunk = false;
	std::set<std::pair<int, int> > searched;
	std::queue<SearchTile> queue;

	auto searchStep = [&](SearchTile searchTile) {
		if (searchTile.dist > 4 || searched.contains(std::pair<int, int>(searchTile.pos))) {
			return;
		}

		searched.insert(std::pair<int, int>(searchTile.pos));

		// If this is a trunk, we've found our trunk!
		auto &tile = ctx.plane.getTile(searchTile.pos);
		if (dynamic_cast<TreeTrunkTrait *>(tile.traits.get())) {
			foundTrunk = true;
		}

		// If it's leaves, continue our search
		else if (dynamic_cast<TreeLeavesTrait *>(tile.traits.get())) {
			queue.push({searchTile.pos + Swan::TilePos{-1, 0}, searchTile.dist + 1});
			queue.push({searchTile.pos + Swan::TilePos{1, 0}, searchTile.dist + 1});
			queue.push({searchTile.pos + Swan::TilePos{0, 1}, searchTile.dist + 1});
			queue.push({searchTile.pos + Swan::TilePos{0, -1}, searchTile.dist + 1});
		}
	};

	searchStep({pos, 0});
	while (!queue.empty() && !foundTrunk) {
		if (foundTrunk) {
			return;
		}

		auto searchTile = queue.front();
		queue.pop();
		searchStep(searchTile);
	}

	if (!foundTrunk) {
		breakTileAndDropItem(ctx, pos);
	}
}

}
