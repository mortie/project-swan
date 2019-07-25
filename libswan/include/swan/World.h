#pragma once

#include <memory>
#include <vector>
#include <string>

#include "common.h"
#include "Tile.h"
#include "WorldPlane.h"
#include "WorldGen.h"
#include "Entity.h"

namespace Swan {

class World {
public:
	WorldPlane::ID addPlane(std::string gen);

	WorldPlane::ID addPlane() {
		return addPlane(default_worldgen_);
	}

	void setCurrentPlane(WorldPlane::ID id) {
		current_plane_ = id;
	}

	WorldPlane &getPlane(WorldPlane::ID id) {
		return planes_[id];
	}

	Tile::ID getTileID(const std::string &name) {
		return tile_map_.getID(name);
	}

	void registerTile(std::shared_ptr<Tile> t) {
		tile_map_.registerTile(t);
	}

	void registerWorldGen(std::shared_ptr<WorldGen::Factory> gen) {
		worldgens_[gen->name_] = gen;
	}

	void registerEntity(std::shared_ptr<Entity::Factory> ent) {
		ents_[ent->name_] = ent;
	}

	void draw(Win &win);
	void update(float dt);
	void tick();

	WorldPlane::ID current_plane_;
	std::vector<WorldPlane> planes_;
	std::string default_worldgen_;

	TileMap tile_map_;
	std::map<std::string, std::shared_ptr<WorldGen::Factory>> worldgens_;
	std::map<std::string, std::shared_ptr<Entity::Factory>> ents_;
};

}
