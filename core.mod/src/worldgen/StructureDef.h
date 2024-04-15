#pragma once

#include <swan/swan.h>
#include <unordered_map>

namespace CoreMod {

class StructureDef {
public:
	struct Meta {
		std::span<int> grassLevels;
		std::span<int> stoneLevels;
		Swan::World &world;
	};

	virtual void generateArea(
		const Meta &meta, Swan::TilePos pos, Swan::Vec2i size,
		std::unordered_map<Swan::TilePos, Swan::Tile::ID> &map) = 0;
};

}
