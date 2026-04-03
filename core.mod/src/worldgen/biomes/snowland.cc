#include "biomes.h"
#include "tiles.h"
#include "worldgen/wgutil.h"

namespace CoreMod::biomes {

static void postProcess(WorldArea &area, WGContext &wg)
{
}

Biome snowland = {
	.name = "snowland",

	.temperature = -5,

	.surfaceTile = tiles::snow,
	.soilTile = tiles::snow,

	.postProcess = postProcess,
};

}
