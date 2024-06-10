#pragma once

#include <swan/swan.h>

namespace CoreMod {

struct PipeConnectibleTileTrait: Swan::Tile::Traits {};

void registerGlassPipe(Swan::Mod &mod);

}
