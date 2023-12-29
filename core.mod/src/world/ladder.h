#include <swan/swan.h>

namespace CoreMod {

struct LadderTileTrait: Swan::Tile::Traits {};

struct RopeLadderTileTrait: LadderTileTrait {
	RopeLadderTileTrait(bool isAnchor, std::string d):
		isAnchor(isAnchor), direction(d)
	{}

	bool isAnchor;
	std::string direction;
};

void spawnRopeLadderAnchor(const Swan::Context &ctx, Swan::TilePos pos);
void cascadeRopeLadder(const Swan::Context &ctx, Swan::TilePos pos);

}
