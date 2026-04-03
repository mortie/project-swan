#include "biomes.h"
#include "tiles.h"
#include "worldgen/prefabs/TreeCrown.h"
#include "worldgen/wgutil.h"

namespace CoreMod::biomes {

static void spawnTree(
	Swan::TilePos pos, WorldArea &area, WGContext &wg)
{
	int height = 3 + Swan::random(Swan::random(pos.x ^ wg.seed)) % 5;
	for (int y = 0; y < height; ++y) {
		auto ap = pos.add(0, -y);
		if (y == 0) {
			area(ap) = tiles::tree__base;
		} else {
			area(ap) = tiles::tree__stem;
		}
	}

	int r = Swan::random(Swan::random(pos.x ^ wg.seed)) % TreeCrown::variants.size();
	const Prefab *crown = TreeCrown::variants[r];
	area.place(*crown, pos.add(-crown->width / 2, -height - crown->height + 1));
}

static void generateTrees(
	WorldArea &area, WGContext &wg)
{
	auto shouldSpawnTree = [&](int x) {
		return Swan::random(x ^ wg.seed) % 4 == 0;
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
		for (int rx = 1; rx <= 6; ++rx) {
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
			if (tileBelow == tiles::grass && tile == Swan::World::AIR_TILE_ID) {
				spawnTree({x, y}, area, wg);
			}
		}
	}
}

void generateTallGrass(WorldArea &area, WGContext &wg)
{
	if (!area.hasSurface) {
		return;
	}

	auto shouldSpawnGrass = [&](int x) {
		return wg.perlin.noise2D(x / 20.6, 0) > 0;
	};

	for (int x = area.begin.x; x < area.end.x; ++x) {
		if (!shouldSpawnGrass(x)) {
			continue;
		}

		for (int y = area.begin.y; y < area.end.y; ++y) {
			Swan::Tile::ID tile = area({x, y});
			Swan::Tile::ID tileBelow = area({x, y + 1});
			if (tileBelow == tiles::grass && tile == Swan::World::AIR_TILE_ID) {
				area({x, y}) = tiles::tallGrass;
			}
		}
	}
}


static void postProcess(WorldArea &area, WGContext &wg)
{
	generateTallGrass(area, wg);
	generateTrees(area, wg);

	generateSurfaceShrubs(area, wg, 8, {
		{tiles::grass, tiles::potatoBush, 2},
		{tiles::grass, tiles::deadShrub, 2},
		{tiles::grass, tiles::boulder, 2},
	});
}

Biome grassland = {
	.name = "grassland",

	.surfaceTile = tiles::grass,
	.soilTile = tiles::dirt,

	.postProcess = postProcess,
};

}
