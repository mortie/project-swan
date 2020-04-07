#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <memory>
#include <SDL2/SDL.h>

#include "Tile.h"
#include "Item.h"
#include "WorldGen.h"
#include "Entity.h"
#include "Resource.h"
#include "OS.h"
#include "util.h"

namespace Swan {

class Mod {
public:
	Mod(const std::string &path, OS::Dynlib &&dynlib):
		path_(path), dynlib_(std::move(dynlib)) {}

	// We have to manually ensure anything created from the dynlib
	// is destructed before the dynlib itself is destructed
	~Mod();

	void init(const std::string &name);

	void registerImage(const std::string &id);
	void registerTile(Tile::Builder tile);
	void registerItem(Item::Builder item);
	void registerWorldGen(const std::string &name, std::unique_ptr<WorldGen::Factory> gen);

	template<typename WG>
	void registerWorldGen(const std::string &name) {
		worldgens_.push_back(WorldGen::Factory{
			.name = name_ + "::" + name,
			.create = [](World &world) {
				return static_cast<std::unique_ptr<WorldGen>>(
					std::make_unique<WG>(world));
			}
		});
	}

	template<typename Ent>
	void registerEntity(const std::string &name) {
		entities_.push_back(Entity::Factory{
			.name = name_ + "::" + name,
			.create = [](const Context &ctx, const Entity::PackObject &obj) {
				return static_cast<std::unique_ptr<Entity>>(
					std::make_unique<Ent>(ctx, obj));
			}
		});
	}

	Iter<std::unique_ptr<ImageResource>> buildImages(SDL_Renderer *renderer);
	Iter<std::unique_ptr<Tile>> buildTiles(const ResourceManager &resources);
	Iter<std::unique_ptr<Item>> buildItems(const ResourceManager &resources);
	Iter<WorldGen::Factory> getWorldGens();
	Iter<Entity::Factory> getEntities();

	std::string name_ = "@uninitialized";

private:
	std::vector<std::string> images_;
	std::vector<Tile::Builder> tiles_;
	std::vector<Item::Builder> items_;
	std::vector<WorldGen::Factory> worldgens_;
	std::vector<Entity::Factory> entities_;

	std::string path_;
	OS::Dynlib dynlib_;
	bool inited_ = false;
};

}
