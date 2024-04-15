#pragma once

#include "StructureDef.h"

#include <stdint.h>

namespace CoreMod {

class TreeDef final: public StructureDef {
public:
	TreeDef(uint32_t seed): seed_(seed) {}

	void generateArea(
		const Meta &meta, Swan::TilePos pos, Swan::Vec2i size,
		std::unordered_map<Swan::TilePos, Swan::Tile::ID> &map) override;

private:
	uint32_t seed_;
};

}
