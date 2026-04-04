#include "biomes.h"
#include "tiles.h"
#include "worldgen/wgutil.h"
#include "worldgen/prefabs/PineCrown.h"

namespace CoreMod::biomes {

static void spawnPine(
		Swan::TilePos pos, WorldArea &area, WGContext &wg)
{
		int height = 1 + Swan::random(Swan::random(pos.x ^ wg.seed)) % 2;
		for (int y = 0; y < height; ++y) {
			auto ap = pos.add(0, -y);
			if (y == 0) {
				area(ap) = tiles::pine__base;
			} else {
				area(ap) = tiles::pine__stem;
			}
		}

		int r = Swan::random(Swan::random(pos.x ^ wg.seed)) % PineCrown::variants.size();
		const Prefab *crown = PineCrown::variants[r];
		area.place(*crown, pos.add(-crown->width / 2, -height - crown->height + 1));
}

static void generatePines(
	WorldArea &area, WGContext &wg)
{
	if (!area.hasSurface) {
		return;
	}

	auto shouldSpawnTree = [&](int x) {
		return Swan::random(x ^ wg.seed) % 30 == 0;
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
			if (tileBelow == tiles::snow && tile == Swan::World::AIR_TILE_ID) {
				spawnPine({x, y}, area, wg);
			}
		}
	}
}

static void postProcess(WorldArea &area, WGContext &wg)
{
	generateOutcrops(area, wg, 16, {5, 55}, {
		{tiles::coalOutcrop, 1},
		{tiles::sulphurOutcrop, 1},
	});

	generateOutcrops(area, wg, 20, {30, 300}, {
		{tiles::ironOutcrop, 2},
		{tiles::copperOutcrop, 3},
	});

	generatePines(area, wg);

	for (int x = area.begin.x; x < area.end.x; ++x) {
		if (Swan::random(x ^ 1324 ^ wg.seed) % 8 != 0) {
			continue;
		}

		Swan::TilePos pos(x, area.surfaceLevel(x));
		auto above = pos.add(0, -1);
		if (area(pos) == tiles::stone && area(above) == Swan::World::AIR_TILE_ID) {
			area(above) = tiles::boulder;
		}
	}
}

Biome snowlands = {
	.name = "snowlands",

	.temperature = -5,

	.surfaceTile = tiles::snow,
	.soilTile = tiles::snow,

	.postProcess = postProcess,
};

}
