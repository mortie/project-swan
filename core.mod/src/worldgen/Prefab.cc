#include "Prefab.h"
#include <unordered_map>

namespace CoreMod {

Prefab::Prefab(Swan::World &world, Map map, Data data)
{
	std::unordered_map<char, Swan::Tile::ID> ids;
	ids[' '] = Swan::World::AIR_TILE_ID;
	for (auto &mapping: map) {
		ids[mapping.symbol] = world.getTileID(mapping.name);
	}

	width = 0;
	height = data.size();
	for (const char *row: data) {
		size_t len = strlen(row);
		if (len > width) {
			width = int(len);
		}
	}

	if (width == 0 || height == 0) {
		Swan::warn << "0-sized prefab";
		width = 0;
		height = 0;
		return;
	}

	tiles = std::make_unique<Swan::Tile::ID[]>(width * height);
	int y = 0;
	for (const char *row: data) {
		for (int x = 0; row[x]; ++x) {

			Swan::Tile::ID id;
			auto it = ids.find(row[x]);
			if (it == ids.end()) {
				Swan::warn << "Unmapped symbol: " << row[x];
				id = Swan::World::INVALID_TILE_ID;
			} else {
				id = it->second;
			}

			tiles[y * width + x] = id;
		}
		y += 1;
	}
}

}
