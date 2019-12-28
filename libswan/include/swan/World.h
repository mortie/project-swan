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
#include "Mod.h"

namespace Swan {

class Game;

class World {
public:
	World(Game *game, unsigned long rand_seed);

	void addMod(std::unique_ptr<Mod> mod);
	void setWorldGen(const std::string &gen);
	void spawnPlayer();

	void setCurrentPlane(WorldPlane &plane);
	WorldPlane &addPlane(const std::string &gen);
	WorldPlane &addPlane() { return addPlane(default_world_gen_); }

	Tile &getTileByID(Tile::ID id);
	Tile::ID getTileID(const std::string &name);
	Tile &getTile(const std::string &name);
	Item &getItem(const std::string &name);

	void draw(Win &win);
	void update(float dt);
	void tick(float dt);

	std::vector<std::unique_ptr<Mod>> mods_;

	// World owns tiles and items, the mod just has Builder objects
	std::vector<std::unique_ptr<Tile>> tiles_;
	std::unordered_map<std::string, Tile::ID> tiles_map_;
	std::unordered_map<std::string, std::unique_ptr<Item>> items_;

	// The mods themselves retain ownership of world gens and entities,
	// the world just has non-owning pointers to them
	std::unordered_map<std::string, WorldGen::Factory *> worldgens_;
	std::unordered_map<std::string, Entity::Factory *> ents_;

	Entity *player_;
	Game *game_;

	std::mt19937 random_;
	ResourceManager resources_;

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
