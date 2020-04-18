#pragma once

#include <memory>
#include <vector>
#include <string>
#include <random>
#include <SDL.h>

#include "common.h"
#include "Item.h"
#include "Tile.h"
#include "WorldPlane.h"
#include "WorldGen.h"
#include "Entity.h"
#include "Collection.h"
#include "Resource.h"
#include "Mod.h"
#include "EventEmitter.h"

namespace Swan {

class Game;

class World {
public:
	World(Game *game, unsigned long rand_seed);

	void addMod(ModWrapper &&mod);
	void setWorldGen(std::string gen);
	void spawnPlayer();

	void setCurrentPlane(WorldPlane &plane);
	WorldPlane &addPlane(const std::string &gen);
	WorldPlane &addPlane() { return addPlane(default_world_gen_); }

	Tile &getTileByID(Tile::ID id) { return *tiles_[id]; }
	Tile::ID getTileID(const std::string &name);
	Tile &getTile(const std::string &name);
	Item &getItem(const std::string &name);

	SDL_Color backgroundColor();
	void draw(Win &win);
	void update(float dt);
	void tick(float dt);

	// Event emitters
	EventEmitter<const Context &, TilePos, Tile &>
	evt_tile_break_;

	// World owns all mods
	std::vector<ModWrapper> mods_;

	// World owns tiles and items, the mod just has Builder objects
	std::vector<std::unique_ptr<Tile>> tiles_;
	std::unordered_map<std::string, Tile::ID> tiles_map_;
	std::unordered_map<std::string, std::unique_ptr<Item>> items_;

	// Mods give us factories to create new world gens and new entity collections
	std::unordered_map<std::string, WorldGen::Factory> worldgen_factories_;
	std::vector<EntityCollection::Factory> ent_coll_factories_;

	BodyTrait::HasBody *player_;
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
