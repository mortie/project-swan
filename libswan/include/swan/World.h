#pragma once

#include <memory>
#include <vector>
#include <string>
#include <random>

#include "common.h"
#include "Item.h"
#include "Tile.h"
#include "WorldPlane.h"
#include "WorldGen.h"
#include "Entity.h"
#include "Resource.h"

namespace Swan {

class Game;

class World {
public:
	World(Game *game, unsigned long rand_seed): game_(game), random_(rand_seed) {}

	WorldPlane &addPlane(const std::string &gen);
	WorldPlane &addPlane() { return addPlane(default_world_gen_); }
	void setCurrentPlane(WorldPlane &plane);
	void setWorldGen(const std::string &gen);
	void spawnPlayer();
	void registerTile(std::shared_ptr<Tile> t);
	void registerItem(std::shared_ptr<Item> i);
	void registerWorldGen(std::shared_ptr<WorldGen::Factory> gen);
	void registerEntity(std::shared_ptr<Entity::Factory> ent);
	void registerImage(std::shared_ptr<ImageResource> i);

	Tile &getTileByID(Tile::ID id);
	Tile::ID getTileID(const std::string &name);
	Tile &getTile(const std::string &name);
	Item &getItem(const std::string &name);
	ImageResource &getImage(const std::string &name);

	void draw(Win &win);
	void update(float dt);
	void tick(float dt);

	std::vector<std::shared_ptr<Tile>> tiles_;
	std::map<std::string, Tile::ID> tiles_map_;
	std::map<std::string, std::shared_ptr<Item>> items_;
	std::map<std::string, std::shared_ptr<WorldGen::Factory>> worldgens_;
	std::map<std::string, std::shared_ptr<Entity::Factory>> ents_;
	std::map<std::string, std::shared_ptr<ImageResource>> images_;
	Entity *player_;
	Game *game_;

	std::mt19937 random_;

private:
	class ChunkRenderer {
	public:
		void tick(WorldPlane &plane, ChunkPos abspos);
	};

	ChunkRenderer chunk_renderer_;
	WorldPlane::ID current_plane_;
	std::vector<WorldPlane> planes_;
	std::string default_world_gen_;
};

}
