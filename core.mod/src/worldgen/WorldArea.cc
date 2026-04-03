#include "WorldArea.h"

namespace CoreMod {

void WorldArea::place(const Prefab &prefab, Swan::Vec2i pos)
{
	for (int y = 0; y < prefab.height; ++y) {
		const Swan::Tile::ID *const *row = &prefab.tiles[y * prefab.width];
		for (int x = 0; x < prefab.width; ++x) {
			get(pos.add(x, y)) = *row[x];
		}
	}
}

}
