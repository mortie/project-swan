#include <swan/swan.h>

namespace CoreMod {

struct LadderTileTrait: Swan::Tile::Traits {};

struct RopeLadderTileTrait: LadderTileTrait {
	RopeLadderTileTrait(bool isAnchor, std::string d):
		isAnchor(isAnchor), direction(std::move(d))
	{}

	bool isAnchor;
	std::string direction;
};

void registerRopeLadder(Swan::Mod &mod);

}
