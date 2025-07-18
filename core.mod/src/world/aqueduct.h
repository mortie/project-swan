#pragma once

#include <swan/swan.h>

namespace CoreMod {

struct AqueductTrait: public Swan::Tile::Traits {
	AqueductTrait() = default;
	AqueductTrait(
		Swan::DirectionSet connectable,
		Swan::DirectionSet connectedTo):
		connectable(connectable), connectedTo(connectedTo)
	{}

	Swan::DirectionSet connectable;
	Swan::DirectionSet connectedTo;
};

void registerAqueduct(Swan::Mod &mod);

}
