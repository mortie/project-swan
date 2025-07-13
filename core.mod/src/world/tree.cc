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

void registerTree(Swan::Mod &mod)
{
	auto treeTrunkTrait = std::make_shared<TreeTrunkTrait>();
	auto treeLeavesTrait = std::make_shared<TreeLeavesTrait>();

	mod.registerItem({
		.name = "wood",
		.image = "core::tiles/tree@11",
	});

	std::pair<const char *, const char *> treeSpecs[] = {
		{"tree::base", "core::tiles/tree@10"},
		{"tree::stem", "core::tiles/tree@11"},
		{"tree::cross", "core::tiles/tree@4"},
		{"tree::top", "core::tiles/tree@1"},
	};
	for (auto [name, image]: treeSpecs) {
		mod.registerTile({
			.name = name,
			.image = image,
			.isSolid = false,
			.isSupportV = std::string_view(name) != "tree::top",
			.isSupportH = false,
			.breakableBy = Swan::Tool::AXE,
			.droppedItem = "core::wood",
			.onSpawn = denyIfFloating,
			.onTileUpdate = breakIfFloating,
			.traits = treeTrunkTrait,
		});
	}

	std::pair<const char *, const char *> leavesSpecs[] = {
		{"tree::leaves", "core::tiles/tree@9"},
		{"tree::leaves::left", "core::tiles/tree@0"},
		{"tree::leaves::right", "core::tiles/tree@2"},
		{"tree::top", "core::tiles/tree@1"},
		{"tree::branch::left", "core::tiles/tree@6"},
		{"tree::branch::right", "core::tiles/tree@8"},
		{"tree::center", "core::tiles/tree@7"},
		{"tree::stub::left", "core::tiles/tree@3"},
		{"tree::stub::right", "core::tiles/tree@5"},
		{"tree::cross", "core::tiles/tree@4"},
	};
	for (auto [name, image]: leavesSpecs) {
		mod.registerTile({
			.name = name,
			.image = image,
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
