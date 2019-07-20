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

	std::vector<Tile *> registered_tiles_;
	std::map<std::string, Tile::TileID> tile_id_map_;

	WorldPlane::PlaneID addPlane();
	void setCurrentPlane(WorldPlane::PlaneID id) { current_plane_ = id; }
	WorldPlane &getPlane(WorldPlane::PlaneID id) { return planes_[id]; }

	Tile::TileID getTileID(const std::string &name) {
		return tile_id_map_[name];
	}

	void registerTile(Tile *t);

	void draw(Win &win);
	void update(float dt);
	void tick();
};

}
