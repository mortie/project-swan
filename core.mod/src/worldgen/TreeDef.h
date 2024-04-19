#pragma once

#include "StructureDef.h"

#include <stdint.h>

namespace CoreMod {

class TreeDef final: public StructureDef {
public:
	TreeDef(Swan::World &world, uint32_t seed):
		tGrass_(world.getTileID("core::grass")),
		tTreeTrunk_(world.getTileID("core::tree-trunk")),
		tTreeLeaves_(world.getTileID("core::tree-leaves")),
		seed_(seed)
	{}

	void generateArea(Area &area) override;

private:
	Swan::Tile::ID tGrass_;
	Swan::Tile::ID tTreeTrunk_;
	Swan::Tile::ID tTreeLeaves_;

	void spawnTree(Swan::TilePos base, Area &area);

	uint32_t seed_;
};

}
