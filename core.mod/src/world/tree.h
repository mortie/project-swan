#pragma once

#include <swan/swan.h>

namespace CoreMod {

struct TreeLeavesTrait: public Swan::Tile::Traits {};
struct TreeTrunkTrait: public Swan::Tile::Traits {};

void breakTreeLeavesIfFloating(const Swan::Context &ctx, Swan::TilePos pos);

}
