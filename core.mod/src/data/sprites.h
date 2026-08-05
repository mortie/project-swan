#pragma once

#include <swan/swan.h>

namespace CoreMod::sprites {

#define X(var, name) extern Cygnet::RenderSprite var;
#include "sprites.x"
#undef X

void init(Swan::World &world);

}
