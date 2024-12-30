#include "OreDef.h"

namespace CoreMod {

void OreDef::generateArea(Area &area)
{
	for (int y = area.begin.y; y <= area.end.y; ++y) {
		if (y < 10 || y > 200) {
			continue;
		}

		for (int x = area.begin.x; x < area.end.x; ++x) {
			if (Swan::random(x + seed_) % 8 != 0) {
				continue;
			}

			auto &id = area({x, y});
			if (id != Swan::World::AIR_TILE_ID) {
				continue;
			}

			if (area({x, y + 1}) == tStone_) {
				id = tCoalOutcrop_.normal;
			}
			else if (area({x - 1, y}) == tStone_) {
				id = tCoalOutcrop_.left;
			}
			else if (area({x + 1, y}) == tStone_) {
				id = tCoalOutcrop_.right;
			}
			else if (area({x, y - 1}) == tStone_) {
				id = tCoalOutcrop_.hanging;
			}
		}
	}
}

}