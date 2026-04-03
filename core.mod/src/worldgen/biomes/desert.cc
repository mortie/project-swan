#include "biomes.h"
#include "tiles.h"
#include "worldgen/wgutil.h"

namespace CoreMod::biomes {

static void postProcess(WorldArea &area, WGContext &wg)
{
	generateSurfaceShrubs(area, wg, 16, {
		{tiles::sand, tiles::deadShrub, 1},
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
