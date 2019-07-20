#pragma once

#include "common.h"
#include "Player.h"
#include "Tile.h"
#include "WorldPlane.h"
#include "WorldPlane.h"
#include "Tile.h"

namespace Swan {

class World {
public:
	Player *player_;

	WorldPlane::PlaneID current_plane_;
	std::vector<WorldPlane> planes_;

	TileMap tile_map_;

	WorldPlane::PlaneID addPlane();
	void setCurrentPlane(WorldPlane::PlaneID id) { current_plane_ = id; }
	WorldPlane &getPlane(WorldPlane::PlaneID id) { return planes_[id]; }

	Tile::TileID getTileID(const std::string &name) {
		return tile_map_.getID(name);
	}

	void registerTile(Tile *t) {
		tile_map_.registerTile(t);
	}

	void draw(Win &win);
	void update(float dt);
	void tick();
};

}
