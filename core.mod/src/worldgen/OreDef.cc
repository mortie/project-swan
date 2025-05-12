#include "OreDef.h"

namespace CoreMod {

void OreDef::generateArea(Area &area)
{
	for (int y = area.begin.y; y <= area.end.y; ++y) {
		if (y < 10 || y > 400) {
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

			auto *set = &tCoalOutcrop_;
			if (y > 35 && Swan::random(x * seed_ * 2) % 32 < 16) {
				set = &tIronOutcrop_;
			} else if (Swan::random(x * seed_) % 32 < 4) {
				set = &tSulphurOutcrop_;
			}

			if (area({x, y + 1}) == tStone_) {
				id = set->normal;
			}
			else if (area({x - 1, y}) == tStone_) {
				id = set->left;
			}
			else if (area({x + 1, y}) == tStone_) {
				id = set->right;
			}
			else if (area({x, y - 1}) == tStone_) {
				id = set->hanging;
			}
		}
	}
}

}
