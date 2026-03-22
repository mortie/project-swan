#pragma once

#include <swan/swan.h>

namespace CoreMod::tiles {

#define X(var, name) extern Swan::Tile::ID var;
#include "tiles.x"
#undef X

void init(Swan::World &world);

}
