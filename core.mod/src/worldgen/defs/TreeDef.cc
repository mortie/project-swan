#include "TreeDef.h"

#include "../prefabs/TreeCrown.h"

namespace CoreMod {

TreeDef::TreeDef(Swan::World &world, uint32_t seed):
	seed_(seed),
	tiles_(world),
	crowns_({
		{world, TreeCrown::map, TreeCrown::variant1},
		{world, TreeCrown::map, TreeCrown::variant2},
		{world, TreeCrown::map, TreeCrown::variant3},
		{world, TreeCrown::map, TreeCrown::variant4},
		{world, TreeCrown::map, TreeCrown::variant5},
	})
{}

void TreeDef::spawnTree(
	Swan::TilePos pos, Area &area)
{
	if (!area.hasSurface) {
		return;
	}

	int height = 3 + Swan::random(Swan::random(pos.x ^ seed_)) % 5;
	for (int y = 0; y < height; ++y) {
		auto ap = pos.add(0, -y);
		if (y == 0) {
			area(ap) = tiles_.treeBase;
		} else {
			area(ap) = tiles_.treeStem;
		}
	}

	int r = Swan::random(Swan::random(pos.x ^ seed_)) % crowns_.size();
	Prefab &crown = crowns_[r];

	area.place(crown, pos.add(-crown.width / 2, -height - crown.height + 1));
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
			if (tileBelow == tiles_.grass && tile == Swan::World::AIR_TILE_ID) {
				spawnTree({x, y}, area);
			}
		}
	}
}

}
