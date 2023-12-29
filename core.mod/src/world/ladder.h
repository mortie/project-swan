#include <swan/swan.h>

namespace CoreMod {

struct LadderTileTrait: Swan::Tile::Traits {};

struct RopeLadderTileTrait: LadderTileTrait {
	RopeLadderTileTrait(bool isAnchor): isAnchor(isAnchor)
	{}

	bool isAnchor;
};

void cascadeRopeLadder(const Swan::Context &ctx, Swan::TilePos pos);

}
