#pragma once

#include "StructureDef.h"

#include <stdint.h>

namespace CoreMod {

class TreeDef final: public StructureDef {
public:
	TreeDef(Swan::World &world, uint32_t seed):
		tGrass_(world.getTileID("core::grass")),
		tWood_(world.getTileID("core::wood")),
		tLeaves_(world.getTileID("core::leaves")),
		seed_(seed)
	{}

	void generateArea(Area &area) override;

private:
	Swan::Tile::ID tGrass_;
	Swan::Tile::ID tWood_;
	Swan::Tile::ID tLeaves_;

	void spawnTree(Swan::TilePos base, Area &area);

	uint32_t seed_;
};

}
