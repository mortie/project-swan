#include "tiles.h"

namespace CoreMod::actions {

#define X(var, name) Swan::Action var;
#include "actions.x"
#undef X

void init(Swan::GameIO &game)
{
	auto &inputs = game.inputs();
#define X(var, name) var = inputs.action(name);
#include "actions.x"
#undef X
}

}
