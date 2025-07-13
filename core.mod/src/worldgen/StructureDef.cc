#include "StructureDef.h"

#include "Prefab.h"

namespace CoreMod {

void StructureDef::Area::place(const Prefab &prefab, Swan::Vec2i pos)
{
	for (int y = 0; y < prefab.height; ++y) {
		Swan::Tile::ID *row = &prefab.tiles[y * prefab.width];
		for (int x = 0; x < prefab.width; ++x) {
			get(pos.add(x, y)) = row[x];
		}
	}
}

}
