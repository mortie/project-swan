#pragma once

#include <swan/swan.h>

namespace CoreMod::actions {

#define X(var, name) extern Swan::Action var;
#include "actions.x"
#undef X

void init(Swan::GameIO &game);

}
