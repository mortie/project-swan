#pragma once

#include "../StructureDef.h"
#include "../Prefab.h"

#include <stdint.h>
#include <vector>

namespace CoreMod {

class TreeDef final: public StructureDef {
public:
	TreeDef(Swan::World &world, uint32_t seed);

	void generateArea(Area &area) override;

private:
	struct Tiles {
		Tiles(Swan::World &world):
			grass(world.getTileID("core::grass")),
			treeBase(world.getTileID("core::tree::base")),
			treeStem(world.getTileID("core::tree::stem")),
			treeCenter(world.getTileID("core::tree::center"))
		{}

		using TID = Swan::Tile::ID;
		TID grass;
		TID treeBase;
		TID treeStem;
		TID treeCenter;
	};

	void spawnTree(Swan::TilePos base, Area &area);

	uint32_t seed_;
	Tiles tiles_;
	std::vector<Prefab> crowns_;
};

}
