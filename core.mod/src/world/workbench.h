#pragma once

#include <swan/swan.h>

namespace CoreMod {

struct WorkbenchTileTrait: Swan::Tile::Traits {};

void registerWorkbench(Swan::Mod &mod);

}
