#pragma once

#include "Tile.h"

namespace Swan {

class TileMap {
public:
	std::vector<Tile *> tiles_;
	std::map<std::string, Tile::ID> id_map_;

	Tile::ID getID(const std::string &name) {
		return id_map_[name];
	}

	Tile *get(Tile::ID id) {
		return tiles_[id];
	}

	void registerTile(Tile *t) {
		Tile::ID id = tiles_.size();
		tiles_.push_back(t);
		id_map_[t->name_] = id;
	}
};

}
