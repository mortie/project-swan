#pragma once

#include <swan/swan.h>

namespace CoreMod {

struct BurnableItemTrait: public Swan::Item::Traits {
	float burnTime = 8;
};

}
