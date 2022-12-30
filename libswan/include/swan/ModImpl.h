#pragma once

#include "Mod.h"
#include "EntityCollectionImpl.h"

namespace Swan {

inline void Mod::registerTile(Tile::Builder tile) {
	tiles_.push_back(tile);
}

inline void Mod::registerItem(Item::Builder item) {
	items_.push_back(item);
}

inline void Mod::registerSprite(std::string sprite) {
	sprites_.push_back(sprite);
}

template<typename WG>
inline void Mod::registerWorldGen(std::string name) {
	worldGens_.push_back(WorldGen::Factory{
		.name = name,
		.create = [](World &world) -> std::unique_ptr<WorldGen> {
			return std::make_unique<WG>(world);
		}
	});
}

template<typename Ent>
inline void Mod::registerEntity(const std::string name) {
	static_assert(
		std::is_move_constructible_v<Ent>,
		"Entities must be movable");
	entities_.push_back(EntityCollection::Factory{
		.name = std::move(name),
		.create = [](std::string name) -> std::unique_ptr<EntityCollection> {
			return std::make_unique<EntityCollectionImpl<Ent>>(std::move(name));
		}
	});
}

}
