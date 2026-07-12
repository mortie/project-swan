#pragma once

#include "swan/common.h"
#include <swan/swan.h>

namespace CoreMod {

void registerSeedEntities(Swan::Mod &mod);

void spawnGrassSeed(Swan::Ctx &ctx, Swan::TilePos pos);
void spawnScorchbloomSeed(Swan::Ctx &ctx, Swan::TilePos pos);

}
