#include "biomes.h"
#include "tiles.h"
#include "worldgen/wgutil.h"

namespace CoreMod::biomes {

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
}

Biome snowland = {
	.name = "snowland",

	.temperature = -5,

	.surfaceTile = tiles::snow,
	.soilTile = tiles::snow,

	.postProcess = postProcess,
};

}
