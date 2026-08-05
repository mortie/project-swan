#include "sprites.h"

namespace CoreMod::sounds {

#define X(var, name) Swan::SoundAsset *var;
#include "sounds.x"
#undef X

void init(Swan::World &world)
{
#define X(var, name) var = world.getSound(name);
#include "sounds.x"
#undef X
}

}
