#pragma once

#include <vector>
#include <map>
#include <string>

#include "common.h"
#include "WorldPlane.h"
#include "Player.h"
#include "Tile.h"

class Game {
public:
	Player *player_;
	WorldPlane *current_plane_;
	std::vector<WorldPlane *> planes_;
	std::vector<Tile> registered_tiles_;
	std::map<std::string, Tile::TileID> tile_id_map_;

	void registerTile(std::string &name, Tile &tile);
	Tile::TileID getTileID(std::string &name);

	void draw(Win &win);
	void update(float dt);
	void tick();
};
