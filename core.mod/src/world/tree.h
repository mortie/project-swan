#pragma once

#include "swan/swan.h"

namespace CoreMod {

struct TreeTrait: Swan::Tile::Traits {};

void spawnTree(const Swan::Context &ctx, Swan::TilePos pos);
void breakTree(const Swan::Context &ctx, Swan::TilePos pos);

}
