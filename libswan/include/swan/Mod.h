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
#include "util.h"

namespace Swan {

class Mod {
public:
	Mod(const std::string &path, SDL_Renderer *renderer): path_(path), renderer_(renderer) {}

	void init(const std::string &name);

	void registerImage(const std::string &name, const std::string &path, int frame_height = -1);

	void registerTile(Tile::Builder tile);
	void registerItem(Item::Builder item);
	void registerWorldGen(const std::string &name, std::unique_ptr<WorldGen::Factory> gen);
	void registerEntity(const std::string &name, std::unique_ptr<Entity::Factory> ent);

	Iter<std::unique_ptr<Tile>> buildTiles(const ResourceManager &resources);
	Iter<std::unique_ptr<Item>> buildItems(const ResourceManager &resources);
	Iter<WorldGen::Factory *> getWorldGens();
	Iter<Entity::Factory *> getEntities();

	std::string name_ = "@uninitialized";

private:
	std::vector<std::unique_ptr<ImageResource>> images_;
	std::vector<Tile::Builder> tiles_;
	std::vector<Item::Builder> items_;
	std::vector<std::unique_ptr<WorldGen::Factory>> worldgens_;
	std::vector<std::unique_ptr<Entity::Factory>> entities_;

	std::string path_;
	SDL_Renderer *renderer_;
	bool inited_ = false;
};

}
