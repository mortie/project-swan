#include "sprites.h"

namespace CoreMod::sprites {

#define X(var, name) Cygnet::RenderSprite var;
#include "sprites.x"
#undef X

void init(Swan::WorldData &world)
{
#define X(var, name) var = world.getSprite(name);
#include "sprites.x"
#undef X
}

}
