#pragma once

#include "../StructureDef.h"
#include "../Prefab.h"

#include <stdint.h>
#include <vector>

namespace CoreMod {

class TreeDef final: public StructureDef {
public:
	TreeDef(Swan::World &world, uint32_t seed);

	void generateArea(WorldArea &area) override;

private:
	void spawnTree(Swan::TilePos base, WorldArea &area);

	uint32_t seed_;
	std::vector<Prefab> crowns_;
};

}
