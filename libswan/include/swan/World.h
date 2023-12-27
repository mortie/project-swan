#pragma once

#include <memory>
#include <vector>
#include <string>
#include <random>
#include <cygnet/Renderer.h>
#include <cygnet/ResourceManager.h>
#include <cygnet/util.h>

#include "common.h"
#include "Item.h"
#include "Tile.h"
#include "WorldPlane.h"
#include "WorldGen.h"
#include "Entity.h"
#include "EntityCollection.h"
#include "Mod.h"
#include "EventEmitter.h"
#include "Recipe.h"

namespace Swan {

class Game;

class World {
public:
	static constexpr Tile::ID INVALID_TILE_ID = 0;
	static constexpr char INVALID_TILE_NAME[] = "@::invalid";
	static constexpr Tile::ID AIR_TILE_ID = 1;
	static constexpr char AIR_TILE_NAME[] = "@::air";

	World(Game *game, unsigned long randSeed, std::vector<std::string> modPaths);
	~World();

	void setWorldGen(std::string gen);
	void spawnPlayer();

	void setCurrentPlane(WorldPlane &plane);
	WorldPlane &addPlane(const std::string &gen);

	WorldPlane &addPlane()
	{
		return addPlane(defaultWorldGen_);
	}

	Tile &getTileByID(Tile::ID id)
	{
		return tiles_[id];
	}

	Tile::ID getTileID(const std::string &name);
	Tile &getTile(const std::string &name);
	Item &getItem(const std::string &name);
	Cygnet::RenderSprite &getSprite(const std::string &name);

	Cygnet::Color backgroundColor();
	void draw(Cygnet::Renderer &rnd);
	void ui();
	void update(float dt);
	void tick(float dt);

	// These things can be used by the mods as they get initialized in the ctor.
	EventEmitter<const Context &, TilePos, Tile &> evtTileBreak_;

	// These things get filled in when the ctor loads mods.
	std::vector<Tile> tiles_;
	std::unordered_map<std::string, Tile::ID> tilesMap_;
	std::unordered_map<std::string, Item> items_;
	std::vector<Recipe> recipes_;
	std::unordered_map<std::string, WorldGen::Factory> worldGenFactories_;
	std::unordered_map<std::string, EntityCollection::Factory> entCollFactories_;

	Game *game_;
	std::mt19937 random_;

	// Mods must be loaded before resources.
	std::vector<ModWrapper> mods_;
	Cygnet::ResourceManager resources_;

	BodyTrait::Body *player_;

private:
	class ChunkRenderer {
public:
		void tick(WorldPlane &plane, ChunkPos abspos);
	};

	std::vector<ModWrapper> loadMods(std::vector<std::string> paths);
	Cygnet::ResourceManager buildResources();

	ChunkRenderer chunkRenderer_;
	WorldPlane::ID currentPlane_;
	std::vector<std::unique_ptr<WorldPlane> > planes_;
	std::string defaultWorldGen_;
};

}
