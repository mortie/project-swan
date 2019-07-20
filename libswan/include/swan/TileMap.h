#pragma once

#include "Tile.h"

namespace Swan {

class TileMap {
public:
	std::vector<Tile *> tiles_;
	std::map<std::string, Tile::TileID> id_map_;

	Tile::TileID getID(const std::string &name) {
		return id_map_[name];
	}

	Tile *get(Tile::TileID id) {
		return tiles_[id];
	}

	void registerTile(Tile *t) {
		Tile::TileID id = tiles_.size();
		tiles_.push_back(t);
		id_map_[t->name_] = id;
	}
};

}
