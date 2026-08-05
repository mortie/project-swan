#pragma once

#include <swan/swan.h>

namespace CoreMod::sounds {

#define X(var, name) extern Swan::SoundAsset *var;
#include "sounds.x"
#undef X

void init(Swan::World &world);

}
