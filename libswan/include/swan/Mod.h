#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <memory>
#include <type_traits>
#include <SDL2/SDL.h>

#include "Tile.h"
#include "Item.h"
#include "WorldGen.h"
#include "Entity.h"
#include "Collection.h"
#include "Resource.h"
#include "OS.h"
#include "util.h"

namespace Swan {

class Mod {
public:
	Mod(std::string name): name_(std::move(name)) {}
	virtual ~Mod() = default;

	void registerImage(const std::string &id);
	void registerTile(Tile::Builder tile);
	void registerItem(Item::Builder item);
	void registerWorldGen(const std::string &name, std::unique_ptr<WorldGen::Factory> gen);

	template<typename WG>
	void registerWorldGen(const std::string &name) {
		worldgens_.push_back(WorldGen::Factory{
			.name = name_ + "::" + name,
			.create = [](World &world) -> std::unique_ptr<WorldGen> {
				return std::make_unique<WG>(world);
			}
		});
	}

	template<typename Ent>
	void registerEntity(const std::string &name) {
		static_assert(
			std::is_move_constructible_v<Ent>,
			"Entities must be movable");
		entities_.push_back(EntityCollection::Factory{
			.name = name_ + "::" + name,
			.create = [](std::string name) -> std::unique_ptr<EntityCollection> {
				return std::make_unique<EntityCollectionImpl<Ent>>(std::move(name));
			}
		});
	}

	const std::string name_;
	std::vector<std::string> images_;
	std::vector<Tile::Builder> tiles_;
	std::vector<Item::Builder> items_;
	std::vector<WorldGen::Factory> worldgens_;
	std::vector<EntityCollection::Factory> entities_;
};

class ModWrapper {
public:
	ModWrapper(std::unique_ptr<Mod> mod, std::string path, OS::Dynlib lib):
		mod_(std::move(mod)), path_(std::move(path)), dynlib_(std::move(lib)) {}

	ModWrapper(ModWrapper &&other) noexcept = default;

	~ModWrapper() {
		// Mod::~Mod will destroy stuff allocated by the dynlib,
		// so we must run its destructor before deleting the dynlib
		mod_.reset();
	}

	Iter<std::unique_ptr<ImageResource>> buildImages(SDL_Renderer *renderer);
	Iter<std::unique_ptr<Tile>> buildTiles(const ResourceManager &resources);
	Iter<std::unique_ptr<Item>> buildItems(const ResourceManager &resources);
	Iter<WorldGen::Factory> getWorldGens();
	Iter<EntityCollection::Factory> getEntities();

	std::unique_ptr<Mod> mod_;
	std::string path_;
	OS::Dynlib dynlib_;
};

}
