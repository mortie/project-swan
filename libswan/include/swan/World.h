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
	WorldPlane &addPlane() { return addPlane(defaultWorldGen_); }

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
	evtTileBreak_;

	// World owns all mods
	std::vector<ModWrapper> mods_;

	// World owns tiles and items, the mod just has Builder objects
	std::vector<std::unique_ptr<Tile>> tiles_;
	std::unordered_map<std::string, Tile::ID> tilesMap_;
	std::unordered_map<std::string, std::unique_ptr<Item>> items_;

	// Mods give us factories to create new world gens and new entity collections
	std::unordered_map<std::string, WorldGen::Factory> worldgenFactories_;
	std::vector<EntityCollection::Factory> entCollFactories_;

	BodyTrait::Body *player_;
	Game *game_;

	std::mt19937 random_;
	ResourceManager resources_;

private:
	class ChunkRenderer {
	public:
		void tick(WorldPlane &plane, ChunkPos abspos);
	};

	ChunkRenderer chunkRenderer_;
	WorldPlane::ID currentPlane_;
	std::vector<std::unique_ptr<WorldPlane>> planes_;
	std::string defaultWorldGen_;
};

}
