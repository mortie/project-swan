#include "tree.h"
#include "world/util.h"

#include <queue>

namespace CoreMod {

void breakTreeLeavesIfFloating(Swan::Ctx &ctx, Swan::TilePos pos)
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
	std::unordered_set<Swan::TilePos> searched;
	std::queue<SearchTile> queue;

	auto searchStep = [&](SearchTile searchTile) {
		if (searchTile.dist > 4 || searched.contains(searchTile.pos)) {
			return;
		}

		searched.insert(searchTile.pos);

		// If this is a trunk, we've found our trunk!
		auto &tile = ctx.plane.tiles().get(searchTile.pos);
		if (dynamic_cast<TreeTrunkTrait *>(tile.traits.get())) {
			foundTrunk = true;
		}

		// If it's leaves, continue our search
		else if (dynamic_cast<TreeLeavesTrait *>(tile.traits.get())) {
			queue.push({searchTile.pos.add(-1, 0), searchTile.dist + 1});
			queue.push({searchTile.pos.add(1, 0), searchTile.dist + 1});
			queue.push({searchTile.pos.add(0, 1), searchTile.dist + 1});
			queue.push({searchTile.pos.add(0, -1), searchTile.dist + 1});
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
