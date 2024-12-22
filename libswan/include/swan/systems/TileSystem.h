#pragma once

#include "../Tile.h"
#include "../common.h"

#include <string_view>
#include <vector>

namespace Swan {

class WorldPlane;

struct Raycast {
	bool hit;
	Tile &tile;
	TilePos pos;
	Vec2i face;
};

class TileSystem {
public:
	TileSystem(WorldPlane &plane): plane_(plane) {}

	void set(TilePos pos, std::string_view name);
	void setID(TilePos pos, Tile::ID id);
	bool setIDWithoutUpdate(TilePos pos, Tile::ID id);

	Tile &get(TilePos pos);
	Tile::ID getID(TilePos pos);

	bool breakTile(TilePos pos);
	bool placeTile(TilePos pos, Tile::ID id);

	Raycast raycast(Vec2 pos, Vec2 direction, float distance);

	void scheduleUpdate(TilePos pos)
	{
		scheduledUpdatesA_.push_back(pos);
	}

private:
	void beginTick();
	void endTick();

	WorldPlane &plane_;

	// Tiles to update the next tick
	std::vector<TilePos> scheduledUpdatesA_;
	std::vector<TilePos> scheduledUpdatesB_;

	friend WorldPlane;
};

}
