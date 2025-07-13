#pragma once

#include <swan/swan.h>
#include <span>

namespace CoreMod {

struct Prefab;

class StructureDef {
public:
	struct Area {
		Swan::TilePos begin;
		Swan::Vec2i end;
		Swan::Tile::ID **rows;
		int *surfaceLevels;
		bool hasSurface = false;

		Swan::Tile::ID &get(Swan::TilePos tp)
		{
			if (
				tp.x < begin.x || tp.y < begin.y ||
				tp.x >= end.x || tp.y >= end.y) {
				dummyTile = Swan::World::AIR_TILE_ID;
				return dummyTile;
			}

			return rows[tp.y - begin.y][tp.x - begin.x];
		}

		Swan::Tile::ID &operator()(Swan::TilePos tp) { return get(tp); }

		int &surfaceLevel(int x)
		{
			if (x < begin.x || x >= end.x) {
				return dummySurfaceLevel;
			}

			return surfaceLevels[x - begin.x];
		}

		void place(const Prefab &prefab, Swan::TilePos pos);

	private:
		Swan::Tile::ID dummyTile;
		int dummySurfaceLevel = 0;
	};

	virtual void generateArea(Area &area) = 0;

protected:
	~StructureDef() = default;
};

}
