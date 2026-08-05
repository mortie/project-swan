#include "tiles.h"

namespace CoreMod::tiles {

#define X(var, name) Swan::Tile::ID var;
#include "tiles.x"
#undef X

void init(Swan::World &world)
{
#define X(var, name) var = world.getTileID(name);
#include "tiles.x"
#undef X
}

}
