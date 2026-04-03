#include "biomes.h"
#include "tiles.h"
#include "worldgen/wgutil.h"

namespace CoreMod::biomes {

static void postProcess(WorldArea &area, WGContext &wg)
{
	generateSurfaceShrubs(area, wg, 16, {
		{tiles::sand, tiles::deadShrub, 1},
	});

	generateOutcrops(area, wg, 10, {5, 55}, {
		{tiles::coalOutcrop, 1},
		{tiles::sulphurOutcrop, 1},
	});

	generateOutcrops(area, wg, 14, {30, 300}, {
		{tiles::ironOutcrop, 3},
		{tiles::copperOutcrop, 2},
	});
}

Biome desert = {
	.name = "desert",

	.humidity = 10,
	.temperature = 40,
	.steepness = 0.2,

	.surfaceTile = tiles::sand,
	.soilTile = tiles::sand,

	.postProcess = postProcess,
};

}
