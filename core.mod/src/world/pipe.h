#pragma once

#include <swan/swan.h>

namespace CoreMod {

struct PipeConnectibleTileTrait: Swan::Tile::Traits {
	PipeConnectibleTileTrait() = default;
	PipeConnectibleTileTrait(Swan::DirectionSet dirs):
		pipeConnectDirections(dirs)
	{}

	Swan::DirectionSet pipeConnectDirections = Swan::DirectionSet::all();
};

void registerGlassPipe(Swan::Mod &mod);

}
