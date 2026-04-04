#pragma once

#include <bitset>
#include <swan/swan.h>

#include "Prefab.h"

namespace CoreMod {

struct WorldArea {
	Swan::TilePos begin;
	Swan::TilePos end;
	Swan::Tile::ID **rows;
	Swan::Tile::ID **backgroundRows;
	int *surfaceLevels;
	bool *sameBiomes;
	bool hasSurface = false;

	Swan::Tile::ID &get(Swan::TilePos tp)
	{
		if (
			tp.x < begin.x || tp.y < begin.y ||
			tp.x >= end.x || tp.y >= end.y
		) {
			dummyTile = Swan::World::AIR_TILE_ID;
			return dummyTile;
		}

		return rows[tp.y - begin.y][tp.x - begin.x];
	}

	Swan::Tile::ID &operator()(Swan::TilePos tp) { return get(tp); }

	Swan::Tile::ID &background(Swan::TilePos tp)
	{
		if (
			tp.x < begin.x || tp.y < begin.y ||
			tp.x >= end.x || tp.y >= end.y
		) {
			dummyTile = Swan::World::AIR_TILE_ID;
			return dummyTile;
		}

		return backgroundRows[tp.y - begin.y][tp.x - begin.x];
	}

	int &surfaceLevel(int x)
	{
		if (x < begin.x || x >= end.x) {
			return dummySurfaceLevel;
		}

		return surfaceLevels[x - begin.x];
	}

	bool isSameBiome(int x)
	{
		if (x < begin.x || x >= end.x) {
			return false;
		}

		return sameBiomes[x - begin.x];
	}

	void place(const Prefab &prefab, Swan::TilePos pos);

private:
	Swan::Tile::ID dummyTile;
	int dummySurfaceLevel = 0;
};

}
