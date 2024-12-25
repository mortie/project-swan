#pragma once

#include <memory>
#include <vector>
#include <string>
#include <random>
#include <span>
#include <cygnet/Renderer.h>
#include <cygnet/ResourceManager.h>
#include <cygnet/util.h>

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
#include "swan.capnp.h"

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
	static constexpr Fluid::ID INVALID_FLUID_ID = 63;
	static constexpr char INVALID_FLUID_NAME[] = "@::invalid";

	static constexpr char INVALID_SOUND_NAME[] = "@::invalid";
	static constexpr char THUD_SOUND_NAME[] = "@::thud";

	World(Game *game, unsigned long randSeed, std::span<std::string> modPaths);
	~World();

	void setWorldGen(std::string gen);
	void spawnPlayer();

	void setCurrentPlane(WorldPlane &plane);

	WorldPlane &currentPlane()
	{
		return *planes_[currentPlane_].plane;
	}

	WorldPlane &addPlane(std::string gen);

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

	Tile::ID getTileID(std::string_view name);
	Tile &getTile(std::string_view name)
	{
		return tiles_[getTileID(name)];
	}

	Item &getItem(std::string_view name);

	Fluid &getFluidByID(Fluid::ID id)
	{
		return fluids_[id];
	}

	Fluid::ID getFluidID(std::string_view name);
	Fluid &getFluid(std::string_view name)
	{
		return fluids_[getFluidID(name)];
	}

	Cygnet::RenderSprite &getSprite(std::string_view name);
	SoundAsset *getSound(std::string_view name);

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

	void serialize(proto::World::Builder w);
	void deserialize(proto::World::Reader r);

	HashMap<std::string> modPaths_;

	// These things get filled in when the ctor loads mods.
	std::vector<Tile> tiles_;
	HashMap<Tile::ID> tilesMap_;
	HashMap<Item> items_;
	std::vector<Fluid> fluids_;
	HashMap<Fluid::ID> fluidsMap_;
	std::vector<Recipe> recipes_;
	HashMap<WorldGen::Factory> worldGenFactories_;
	HashMap<EntityCollection::Factory> entCollFactories_;

	Game *game_;
	std::mt19937 random_;

	// Mods must be loaded before resources.
	std::vector<ModWrapper> mods_;
	Cygnet::ResourceManager resources_;
	HashMap<SoundAsset> sounds_;

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

	std::vector<ModWrapper> loadMods(std::span<std::string> paths);
	void buildResources();

	ChunkRenderer chunkRenderer_;
	WorldPlane::ID currentPlane_ = 0;
	std::vector<PlaneWrapper> planes_;
	std::string defaultWorldGen_;

	Item *invalidItem_;
};

}
