#pragma once

#include <swan/log.h>
#include <cygnet/ResourceManager.h>

#include "Tile.h"
#include "Item.h"
#include "Fluid.h"
#include "WorldGen.h"
#include "EntityCollection.h"
#include "Mod.h"
#include "Recipe.h"
#include "cygnet/Renderer.h"

namespace Swan {

class WorldData {
public:
	WorldData() = default;
	WorldData(WorldData &&) = default;
	~WorldData();

	WorldData &operator=(WorldData &&) = default;

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

	/**
	 * Tiles
	 */

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

	Tile &invalidTile()
	{
		return tiles_[INVALID_TILE_ID];
	}

	/**
	 * Items
	 */

	Item &getItem(std::string_view name);
	Item &getItemByID(Tile::ID id)
	{
		if (id >= items_.size()) {
			warn << "Invalid ID: " << id;
			return items_[INVALID_TILE_ID];
		}

		return items_[id];
	}

	Item &invalidItem()
	{
		return items_[INVALID_TILE_ID];
	}

	/**
	 * Fluids
	 */

	Fluid &getFluidByID(Fluid::ID id)
	{
		return fluids_[id];
	}

	Fluid::ID getFluidID(std::string_view name);
	Fluid &getFluid(std::string_view name)
	{
		return fluids_[getFluidID(name)];
	}

	Fluid &invalidFluid()
	{
		return fluids_[INVALID_FLUID_ID];
	}

	/**
	 * Etc
	 */

	Cygnet::RenderSprite &getSprite(std::string_view name);
	SoundAsset *getSound(std::string_view name);

	std::span<Recipe> getRecipes(std::string_view kind)
	{
		auto it = recipes_.find(kind);
		if (it == recipes_.end()) {
			warn << "Attempt to access unknown recipe kind " << kind;
			return {};
		}

		return it->second;
	}

	void loadMods(std::span<const std::string> paths);
	void buildResources(Cygnet::Renderer &rnd, std::vector<std::string> namesByID);

	// These things get filled in when we load mods.
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
	std::vector<std::string> namesByID_;

	// Mods must be loaded before resources.
	std::vector<ModWrapper> mods_;
	Cygnet::ResourceManager resources_;
	HashMap<SoundAsset> sounds_;
};

}
