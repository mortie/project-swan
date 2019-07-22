#pragma once

#include <memory>
#include <vector>
#include <string>

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

	WorldGen::ID default_worldgen_;
	std::vector<std::shared_ptr<WorldGen::Factory>> worldgens_;
	TileMap tile_map_;

	WorldPlane::ID addPlane(WorldGen::ID gen);
	WorldPlane::ID addPlane() { return addPlane(default_worldgen_); }
	void setCurrentPlane(WorldPlane::ID id) { current_plane_ = id; }
	WorldPlane &getPlane(WorldPlane::ID id) { return planes_[id]; }

	Tile::ID getTileID(const std::string &name) {
		return tile_map_.getID(name);
	}

	void registerTile(std::shared_ptr<Tile> t) {
		tile_map_.registerTile(t);
	}

	void registerWorldGen(std::shared_ptr<WorldGen::Factory> gen) {
		worldgens_.push_back(gen);
	}

	void draw(Win &win);
	void update(float dt);
	void tick();
};

}
