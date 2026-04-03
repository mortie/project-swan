#include "tiles.h"
#include "biomes.h"
#include "swan/common.h"
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

static void generateLakes(
	WorldArea &area, WGContext &wg)
{
	auto isLake = [&](Swan::TilePos pos) {
		int surface = area.surfaceLevel(pos.x);
		return
			pos.y >= surface &&
			pos.y <= surface + 20 &&
			wg.perlin.noise2D(pos.x / 32.6, pos.y / 21.565) > 0.4;
	};

	// Recursive lambdas need to take themselves as a parameter
	auto isClayBase = [&](const auto &self, Swan::TilePos pos) {
		bool hasLake =
			isLake(pos.add(-1, -1)) ||
			isLake(pos.add(1, -1)) ||
			(isLake(pos.add(1, 0)) && area(pos.add(-1, 0)) != Swan::World::AIR_TILE_ID);
		if (hasLake && Swan::random(pos.x) % 32 < 31) {
			return true;
		}

		uint32_t rand = Swan::random((pos.x << 5) ^ pos.y) % 16;
		if (rand < 14) {
			return false;
		}

		if (isLake(pos.add(0, -1))) {
			return true;
		}

		if (self(self, pos.add(0, -1))) {
			return true;
		}

		if (rand < 14) {
			return false;
		}

		if (self(self, pos.add(-1, -1))) {
			return true;
		}

		if (self(self, pos.add(1, -1))) {
			return true;
		}

		return false;
	};

	auto isClay = [&](Swan::TilePos pos) {
		return isClayBase(isClayBase, pos);
	};

	// Generate lake
	for (int y = area.begin.y; y <= area.end.y; ++y) {
		for (int x = area.begin.x; x <= area.end.x; ++x) {
			Swan::TilePos pos = {x, y};

			if (isLake(pos)) {
				area(pos) = tiles::water;
			} else if (isClay(pos)) {
				area(pos) = tiles::clayTile;
			}
		}
	}
}

static void generateTrees(
	WorldArea &area, WGContext &wg)
{
	if (!area.hasSurface) {
		return;
	}

	auto shouldSpawnTree = [&](int x) {
		return Swan::random(x ^ wg.seed) % 4 == 0;
	};

	for (int x = area.begin.x; x < area.end.x; ++x) {
		if (!shouldSpawnTree(x)) {
			continue;
		}

		// Avoid generating trees near biome boundaries
		if (!area.isSameBiome(x - 5) || !area.isSameBiome(x + 5)) {
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
	generateLakes(area, wg);
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
