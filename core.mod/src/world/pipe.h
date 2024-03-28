#include <swan/swan.h>

namespace CoreMod {

struct PipeConnectableTileTrait: Swan::Tile::Traits {};

struct PipeTileTrait: PipeConnectableTileTrait {
	PipeTileTrait(std::string p): prefix(std::move(p)) {}
	std::string prefix;
};

void registerGlassPipe(Swan::Mod &mod);

}
