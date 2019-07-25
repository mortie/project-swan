#pragma once

#include <memory>

#include "Tile.h"

namespace Swan {

class TileMap {
public:
	Tile::ID getID(const std::string &name) {
		return id_map_[name];
	}

	Tile &get(Tile::ID id) {
		if (id >= tiles_.size())
			return Tile::invalid_tile;
		return *tiles_[id];
	}

	void registerTile(std::shared_ptr<Tile> t) {
		Tile::ID id = tiles_.size();
		tiles_.push_back(t);
		id_map_[t->name_] = id;
	}

private:
	std::vector<std::shared_ptr<Tile>> tiles_;
	std::map<std::string, Tile::ID> id_map_;
};

}
