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

class TileSystemImpl {
public:
	TileSystemImpl(WorldPlane &plane): plane_(plane) {}

	/*
	 * Available to game logic
	 */

	void set(TilePos pos, std::string_view name);
	void setID(TilePos pos, Tile::ID id);
	bool setIDWithoutUpdate(TilePos pos, Tile::ID id);

	Tile &get(TilePos pos);
	Tile::ID getID(TilePos pos);
	uint8_t getLightLevel(TilePos pos);

	bool breakTile(TilePos pos);
	bool placeTile(TilePos pos, Tile::ID id);

	Raycast raycast(Vec2 pos, Vec2 direction, float distance);

	void scheduleUpdate(TilePos pos)
	{
		scheduledUpdatesA_.push_back(pos);
	}

	void spawnTileParticles(TilePos pos, const Tile &tile);

	/*
	 * Available to friends
	 */

	void beginTick();
	void endTick();

private:
	WorldPlane &plane_;

	// Tiles to update the next tick
	std::vector<TilePos> scheduledUpdatesA_;
	std::vector<TilePos> scheduledUpdatesB_;

	friend WorldPlane;
};

class TileSystem: private TileSystemImpl {
public:
	using TileSystemImpl::TileSystemImpl;
	using TileSystemImpl::set;
	using TileSystemImpl::setID;
	using TileSystemImpl::setIDWithoutUpdate;
	using TileSystemImpl::get;
	using TileSystemImpl::getID;
	using TileSystemImpl::getLightLevel;
	using TileSystemImpl::breakTile;
	using TileSystemImpl::placeTile;
	using TileSystemImpl::raycast;
	using TileSystemImpl::scheduleUpdate;
	using TileSystemImpl::spawnTileParticles;

	friend WorldPlane;
};

}
