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

	WorldPlane *current_plane_;
	std::vector<WorldPlane *> planes_;

	std::vector<Tile> registered_tiles_;
	std::map<std::string, Tile::TileID> tile_id_map_;

	Tile::TileID getTileID(const std::string &name) {
		return tile_id_map_[name];
	}

	void draw(Win &win);
	void update(float dt);
	void tick();
};

}
