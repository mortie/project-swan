#pragma once

#include <swan/swan.h>

namespace CoreMod {

struct TreeLeavesTrait: public Swan::Tile::Traits {};
struct TreeTrunkTrait: public Swan::Tile::Traits {};

void breakTreeLeavesIfFloating(Swan::Ctx &ctx, Swan::TilePos pos);
void registerTree(Swan::Mod &mod);

}
