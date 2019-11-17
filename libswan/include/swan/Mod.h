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

namespace Swan {

class Mod {
public:
	void init(const std::string &name);

	void registerImage(const std::string &name, const std::string &path, int frame_height = -1);

	void registerTile(const Tile::Builder &tile);
	void registerItem(const Item::Builder &item);
	void registerWorldGen(const std::string &name, std::unique_ptr<WorldGen::Factory> gen);
	void registerEntity(const std::string &name, std::unique_ptr<Entity::Factory> ent);

	std::string name_;
	std::string path_;

	std::unordered_map<std::string, std::unique_ptr<ImageResource>> images_;
	std::unordered_map<std::string, Tile::Builder> tiles_;
	std::unordered_map<std::string, Item::Builder> items_;
	std::unordered_map<std::string, std::unique_ptr<WorldGen::Factory>> worldgens_;
	std::unordered_map<std::string, std::unique_ptr<Entity::Factory>> entities_;
	SDL_Renderer *renderer_;
	bool inited_ = false;
};

}
