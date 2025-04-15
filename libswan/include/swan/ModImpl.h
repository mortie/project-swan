#pragma once

#include "Mod.h"
#include "EntityCollectionImpl.h"
#include "util.h"

namespace Swan {

inline void Mod::registerTile(Tile::Builder &&tile)
{
	tiles_.push_back(std::move(tile));
}

inline void Mod::registerItem(Item::Builder &&item)
{
	items_.push_back(std::move(item));
}

inline void Mod::registerFluid(Fluid::Builder &&fluid)
{
	fluids_.push_back(std::move(fluid));
}

inline void Mod::registerRecipe(Recipe::Builder &&recipe)
{
	recipes_.push_back(std::move(recipe));
}

inline void Mod::registerSprite(std::string &&sprite)
{
	sprites_.push_back(std::move(sprite));
}

inline void Mod::registerSound(std::string &&sound)
{
	sounds_.push_back(std::move(sound));
}

inline void Mod::registerRecipeKind(std::string &&kind)
{
	recipeKinds_.push_back(std::move(kind));
}

template<typename WG>
inline void Mod::registerWorldGen(std::string name)
{
	worldGens_.push_back(WorldGen::Factory{
		.name = cat(name_, "::", name),
		.create = [](World &world) -> std::unique_ptr<WorldGen> {
			return std::make_unique<WG>(world);
		}
	});
}

template<typename Ent>
inline void Mod::registerEntity(const std::string name)
{
	static_assert(
		std::is_move_constructible_v<Ent>,
		"Entities must be movable");
	entities_.push_back(EntityCollection::Factory{
		.name = cat(name_, "::", std::move(name)),
		.create = [](std::string name) -> std::unique_ptr<EntityCollection> {
			return std::make_unique<EntityCollectionImpl<Ent>>(std::move(name));
		}
	});
}

}
