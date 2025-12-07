#pragma once

#include <memory>
#include <unordered_set>
#include <vector>
#include <string>
#include <span>
#include <cygnet/Renderer.h>
#include <cygnet/ResourceManager.h>
#include <cygnet/util.h>
#include <swan/log.h>

#include "Clock.h"
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

	struct TickProgress {
		bool ongoing = false;
		std::unordered_set<WorldPlane::ID> tickedPlanes;
	};

	World(Game *game, uint32_t seed, std::span<const std::string> modPaths);
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
	Item &getItemByID(Tile::ID id)
	{
		if (id >= items_.size()) {
			warn << "Invalid ID: " << id;
			return items_[INVALID_TILE_ID];
		}

		return items_[id];
	}

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
	bool tick(float dt, RTDeadline deadline);

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

	std::span<Recipe> getRecipes(std::string_view kind)
	{
		auto it = recipes_.find(kind);
		if (it == recipes_.end()) {
			warn << "Attempt to access unknown recipe kind " << kind;
			return {};
		}

		return it->second;
	}

	uint32_t seed() const { return seed_; }

	void serialize(proto::World::Builder w);
	void deserialize(proto::World::Reader r);

	// These things get filled in when the ctor loads mods.
	std::vector<Tile> tiles_;
	HashMap<Tile::ID> tilesMap_;
	std::vector<Item> items_;
	HashMap<Tile::ID> itemsMap_;
	std::vector<Fluid> fluids_;
	HashMap<Fluid::ID> fluidsMap_;
	HashMap<std::vector<Recipe>> recipes_;
	HashMap<WorldGen::Factory> worldGenFactories_;
	HashMap<EntityCollection::Factory> entCollFactories_;
	HashMap<Cygnet::RenderSprite> sprites_;
	std::vector<ActionSpec> actions_;

	Game *game_;

	// Mods must be loaded before resources.
	std::vector<ModWrapper> mods_;
	Cygnet::ResourceManager resources_;
	HashMap<SoundAsset> sounds_;
	int resourceTickCounter_ = 0;

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

	std::vector<ModWrapper> loadMods(std::span<const std::string> paths);
	void buildResources();

	uint32_t seed_;
	ChunkRenderer chunkRenderer_;
	WorldPlane::ID currentPlane_ = 0;
	std::vector<PlaneWrapper> planes_;
	std::string defaultWorldGen_;
	TickProgress tickProgress_;

	Item *invalidItem_;
};

}
