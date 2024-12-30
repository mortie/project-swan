#pragma once

#include <swan/swan.h>

#include "StructureDef.h"

namespace CoreMod {

class OreDef final: public StructureDef {
public:
	OreDef(Swan::World &world, uint32_t seed):
		tCoalOutcrop_(world, "core::coal-outcrop"),
		tStone_(world.getTileID("core::stone")),
		seed_(seed)
	{}

	void generateArea(Area &area) override;

private:
	struct OutcropSet {
		OutcropSet(Swan::World &world, const char *prefix):
			normal(world.getTileID(prefix)),
			hanging(world.getTileID(Swan::cat(prefix, "::hanging"))),
			left(world.getTileID(Swan::cat(prefix, "::left"))),
			right(world.getTileID(Swan::cat(prefix, "::right")))
		{}

		Swan::Tile::ID normal;
		Swan::Tile::ID hanging;
		Swan::Tile::ID left;
		Swan::Tile::ID right;
	};

	OutcropSet tCoalOutcrop_;
	Swan::Tile::ID tStone_;

	uint32_t seed_;
};

}
