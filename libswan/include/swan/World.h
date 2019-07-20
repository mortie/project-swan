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

	WorldPlane::ID current_plane_;
	std::vector<WorldPlane> planes_;

	TileMap tile_map_;

	WorldPlane::ID addPlane();
	void setCurrentPlane(WorldPlane::ID id) { current_plane_ = id; }
	WorldPlane &getPlane(WorldPlane::ID id) { return planes_[id]; }

	Tile::ID getTileID(const std::string &name) {
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
