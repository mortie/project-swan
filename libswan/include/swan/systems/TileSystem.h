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
	void setBackgroundID(TilePos pos, Tile::ID id);
	bool setBackgroundIDWithoutUpdate(TilePos pos, Tile::ID id);

	Tile &get(TilePos pos);
	Tile *maybeGet(TilePos pos);
	Tile::ID getID(TilePos pos);
	Tile &getBackground(TilePos pos);
	Tile *maybeGetBackground(TilePos pos);
	Tile::ID getBackgroundID(TilePos pos);
	uint8_t getLightLevel(TilePos pos);

	bool breakTile(TilePos pos);
	bool breakTileSilently(TilePos pos);
	bool placeTile(TilePos pos, Tile::ID id);

	Raycast raycast(Vec2 pos, Vec2 direction, float distance);

	void scheduleUpdate(TilePos pos)
	{
		scheduledUpdatesA_.push_back(pos);
	}

	void scheduleUpdateAround(TilePos pos)
	{
		scheduledUpdatesA_.push_back(pos);
		scheduledUpdatesA_.push_back(pos.add(-1, 0));
		scheduledUpdatesA_.push_back(pos.add(1, 0));
		scheduledUpdatesA_.push_back(pos.add(0, -1));
		scheduledUpdatesA_.push_back(pos.add(0, 1));
	}

	void scheduleBackgroundUpdate(TilePos pos)
	{
		scheduledBackgroundUpdatesA_.push_back(pos);
	}

	void scheduleBackgroundUpdateAround(TilePos pos)
	{
		scheduledBackgroundUpdatesA_.push_back(pos);
		scheduledBackgroundUpdatesA_.push_back(pos.add(-1, 0));
		scheduledBackgroundUpdatesA_.push_back(pos.add(1, 0));
		scheduledBackgroundUpdatesA_.push_back(pos.add(0, -1));
		scheduledBackgroundUpdatesA_.push_back(pos.add(0, 1));
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

	// Background tiles to update the next tick
	std::vector<TilePos> scheduledBackgroundUpdatesA_;
	std::vector<TilePos> scheduledBackgroundUpdatesB_;

	friend WorldPlane;
};

class TileSystem: private TileSystemImpl {
public:
	using TileSystemImpl::TileSystemImpl;
	using TileSystemImpl::set;
	using TileSystemImpl::setID;
	using TileSystemImpl::setIDWithoutUpdate;
	using TileSystemImpl::setBackgroundID;
	using TileSystemImpl::setBackgroundIDWithoutUpdate;
	using TileSystemImpl::get;
	using TileSystemImpl::maybeGet;
	using TileSystemImpl::getID;
	using TileSystemImpl::getBackground;
	using TileSystemImpl::maybeGetBackground;
	using TileSystemImpl::getBackgroundID;
	using TileSystemImpl::getLightLevel;
	using TileSystemImpl::breakTile;
	using TileSystemImpl::breakTileSilently;
	using TileSystemImpl::placeTile;
	using TileSystemImpl::raycast;
	using TileSystemImpl::scheduleUpdate;
	using TileSystemImpl::spawnTileParticles;

	friend WorldPlane;
};

}
