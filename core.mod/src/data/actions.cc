#include "tiles.h"

namespace CoreMod::actions {

#define X(var, name) Swan::Action var;
#include "actions.x"
#undef X

void init(Swan::World &world)
{
	auto &game = *world.game_;
#define X(var, name) var = game.action(name);
#include "actions.x"
#undef X
}

}
