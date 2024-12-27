#pragma once

#include <swan/swan.h>
#include <span>

namespace CoreMod {

class StructureDef {
public:
	struct Area {
		Swan::TilePos begin;
		Swan::Vec2i end;
		Swan::Tile::ID **rows;

		Swan::Tile::ID &operator()(Swan::TilePos tp)
		{
			if (
				tp.x < begin.x || tp.y < begin.y ||
				tp.x >= end.x || tp.y >= end.y) {
				dummy = Swan::World::AIR_TILE_ID;
				return dummy;
			}

			return rows[tp.y - begin.y][tp.x - begin.x];
		}

	private:
		Swan::Tile::ID dummy;
	};

	virtual void generateArea(Area &area) = 0;

protected:
	~StructureDef() = default;
};

}
