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
	WorldPlane &addPlane(std::string gen);
	WorldPlane &addPlane() { return addPlane(default_world_gen_); }
	void setCurrentPlane(WorldPlane &plane);
	void setWorldGen(const std::string &gen);
	void spawnPlayer();
	void registerTile(std::shared_ptr<Tile> t);
	void registerWorldGen(std::shared_ptr<WorldGen::Factory> gen);
	void registerEntity(std::shared_ptr<Entity::Factory> ent);

	void draw(Win &win);
	void update(float dt);
	void tick();

	std::map<std::string, std::shared_ptr<WorldGen::Factory>> worldgens_;
	std::map<std::string, std::shared_ptr<Entity::Factory>> ents_;
	TileMap tile_map_;

private:
	Entity *player_;
	WorldPlane::ID current_plane_;
	std::vector<WorldPlane> planes_;
	std::string default_world_gen_;
};

}
