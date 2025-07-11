#pragma once

#include <swan/util.h>
#include "Mod.h"
#include "EntityCollectionImpl.h"

namespace Swan {

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
