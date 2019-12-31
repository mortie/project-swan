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

	void registerImage(ImageResource::Builder image);
	void registerTile(Tile::Builder tile);
	void registerItem(Item::Builder item);
	void registerWorldGen(const std::string &name, std::unique_ptr<WorldGen::Factory> gen);
	void registerEntity(const std::string &name, std::unique_ptr<Entity::Factory> ent);

	Iter<std::unique_ptr<ImageResource>> buildImages(SDL_Renderer *renderer);
	Iter<std::unique_ptr<Tile>> buildTiles(const ResourceManager &resources);
	Iter<std::unique_ptr<Item>> buildItems(const ResourceManager &resources);
	Iter<WorldGen::Factory *> getWorldGens();
	Iter<Entity::Factory *> getEntities();

	std::string name_ = "@uninitialized";

private:
	std::vector<ImageResource::Builder> images_;
	std::vector<Tile::Builder> tiles_;
	std::vector<Item::Builder> items_;
	std::vector<std::unique_ptr<WorldGen::Factory>> worldgens_;
	std::vector<std::unique_ptr<Entity::Factory>> entities_;

	std::string path_;
	OS::Dynlib dynlib_;
	bool inited_ = false;
};

}
