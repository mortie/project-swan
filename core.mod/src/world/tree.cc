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
		if (searchTile.dist > 5 || searched.contains(searchTile.pos)) {
			return;
		}

		searched.insert(searchTile.pos);

		// If this is a trunk, we've found our trunk!
		auto &tile = ctx.plane.tiles().get(searchTile.pos);
		if (dynamic_cast<TreeTrunkTrait *>(tile.more->traits.get())) {
			foundTrunk = true;
		}

		// If it's leaves, continue our search
		else if (dynamic_cast<TreeLeavesTrait *>(tile.more->traits.get())) {
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

void registerTree(Swan::Mod &mod)
{
	auto treeTrunkTrait = std::make_shared<TreeTrunkTrait>();
	auto treeLeavesTrait = std::make_shared<TreeLeavesTrait>();

	mod.registerItem({
		.name = "wood",
		.image = "core::tiles/flora/tree@11",
	});

	for (auto base: {"tree", "pine"}) {
		std::pair<const char *, int> treeSpecs[] = {
			{"base", 10},
			{"stem", 11},
			{"cross", 4},
			{"top", 1},
			{"center", 7},
		};
		for (auto [name, image]: treeSpecs) {
			mod.registerTile({
				.name = Swan::cat(base, "::", name),
				.image = Swan::cat("core::tiles/flora/", base, "@", image),
				.isSolid = false,
				.isSupportV = std::string_view(name) != "top",
				.isSupportH = false,
				.breakableBy = Swan::Tool::AXE,
				.droppedItem = "core::wood",
				.onSpawn = denyIfFloating,
				.onTileUpdate = breakIfFloating,
				.traits = treeTrunkTrait,
			});
		}

		std::pair<const char *, int> leavesSpecs[] = {
			{"leaves", 9},
			{"leaves::left", 0},
			{"leaves::right", 2},
			{"branch::left", 6},
			{"branch::right", 8},
			{"stub::left", 3},
			{"stub::right", 5},
		};
		for (auto [name, image]: leavesSpecs) {
			mod.registerTile({
				.name = Swan::cat(base, "::", name),
				.image = Swan::cat("core::tiles/flora/", base, "@", image),
				.isSolid = false,
				.breakableBy = Swan::Tool::HAND,
				.stepSound = "core::step/grass",
				.placeSound = "core::place/leaves",
				.onBreak = dropRandomItemCount<"core::stick">,
				.onTileUpdate = breakTreeLeavesIfFloating,
				.traits = treeLeavesTrait,
			});
		}
	}
}

}
