#pragma once

#include <memory>
#include <vector>
#include <string>
#include <random>
#include <cygnet/Renderer.h>
#include <cygnet/ResourceManager.h>
#include <cygnet/util.h>
#include <sbon.h>

#include "Fluid.h"
#include "common.h"
#include "Item.h"
#include "Tile.h"
#include "WorldPlane.h"
#include "WorldGen.h"
#include "Entity.h"
#include "EntityCollection.h"
#include "Mod.h"
#include "Recipe.h"
#include "assets.h"
#include "log.h"

namespace Swan {

class Game;

class World {
public:
	static constexpr Tile::ID INVALID_TILE_ID = 0;
	static constexpr char INVALID_TILE_NAME[] = "@::invalid";
	static constexpr Tile::ID AIR_TILE_ID = 1;
	static constexpr char AIR_TILE_NAME[] = "@::air";

	static constexpr char INVALID_SPRITE_NAME[] = "@::invalid";

	static constexpr Fluid::ID AIR_FLUID_ID = 0;
	static constexpr char AIR_FLUID_NAME[] = "@::air";
	static constexpr Fluid::ID SOLID_FLUID_ID = 1;
	static constexpr char SOLID_FLUID_NAME[] = "@::solid";
	static constexpr Fluid::ID INVALID_FLUID_ID = 255;
	static constexpr char INVALID_FLUID_NAME[] = "@::invalid";

	static constexpr char INVALID_SOUND_NAME[] = "@::invalid";
	static constexpr char THUD_SOUND_NAME[] = "@::thud";

	World(Game *game, unsigned long randSeed, std::vector<std::string> modPaths);
	~World();

	void setWorldGen(std::string gen);
	void spawnPlayer();

	void setCurrentPlane(WorldPlane &plane);

	WorldPlane &currentPlane()
	{
		return *planes_[currentPlane_].plane;
	}

	WorldPlane &addPlane(const std::string &gen);

	WorldPlane &addPlane()
	{
		return addPlane(defaultWorldGen_);
	}

	Tile &getTileByID(Tile::ID id)
	{
		if (id >= tiles_.size()) {
			warn << "Invalid ID: " << id;
			return tiles_[INVALID_TILE_ID];
		}

		return tiles_[id];
	}

	Tile::ID getTileID(const std::string &name);
	Tile &getTile(const std::string &name)
	{
		return tiles_[getTileID(name)];
	}

	Item &getItem(const std::string &name);

	Fluid &getFluidByID(Fluid::ID id)
	{
		return fluids_[id];
	}

	Fluid::ID getFluidID(const std::string &name);
	Fluid &getFluid(const std::string &name)
	{
		return fluids_[getFluidID(name)];
	}

	Cygnet::RenderSprite &getSprite(const std::string &name);
	SoundAsset *getSound(const std::string &name);

	Cygnet::Color backgroundColor();
	void draw(Cygnet::Renderer &rnd);
	void update(float dt);
	void tick(float dt);

	Tile &invalidTile()
	{
		return tiles_[INVALID_TILE_ID];
	}

	Item &invalidItem()
	{
		return *invalidItem_;
	}

	Fluid &invalidFluid()
	{
		return fluids_[INVALID_FLUID_ID];
	}

	void serialize(sbon::Writer w);
	void deserialize(sbon::Reader r);

	std::unordered_map<std::string, std::string> modPaths_;

	// These things get filled in when the ctor loads mods.
	std::vector<Tile> tiles_;
	std::unordered_map<std::string, Tile::ID> tilesMap_;
	std::unordered_map<std::string, Item> items_;
	std::vector<Fluid> fluids_;
	std::unordered_map<std::string, Fluid::ID> fluidsMap_;
	std::vector<Recipe> recipes_;
	std::unordered_map<std::string, WorldGen::Factory> worldGenFactories_;
	std::unordered_map<std::string, EntityCollection::Factory> entCollFactories_;

	Game *game_;
	std::mt19937 random_;

	// Mods must be loaded before resources.
	std::vector<ModWrapper> mods_;
	Cygnet::ResourceManager resources_;
	std::unordered_map<std::string, SoundAsset> sounds_;

	EntityRef playerRef_;
	BodyTrait::Body *player_;

private:
	class ChunkRenderer {
	public:
		void tick(WorldPlane &plane, ChunkPos abspos);
	};

	struct PlaneWrapper {
		std::string worldGen;
		std::unique_ptr<WorldPlane> plane;
	};

	std::vector<ModWrapper> loadMods(std::vector<std::string> paths);
	void buildResources();

	ChunkRenderer chunkRenderer_;
	WorldPlane::ID currentPlane_ = 0;
	std::vector<PlaneWrapper> planes_;
	std::string defaultWorldGen_;

	Item *invalidItem_;
};

}
