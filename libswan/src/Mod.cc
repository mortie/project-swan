#include "Mod.h"

#include <stdio.h>
#include <algorithm>

#include "util.h"
#include "log.h"

namespace Swan {

Mod::~Mod() {
	images_.clear();
	tiles_.clear();
	items_.clear();
	worldgens_.clear();
	entities_.clear();
}

void Mod::init(const std::string &name) {
	name_ = name;
	inited_ = true;

	info << "Mod initing: " << name_;
}

void Mod::registerImage(const std::string &id) {
	images_.push_back(name_ + "/" + id);
	info << "  Adding image: " << images_.back();
}

void Mod::registerTile(Tile::Builder tile) {
	tile.name = name_ + "::" + tile.name;
	tiles_.push_back(tile);
	info << "  Adding tile: " << tile.name;
}

void Mod::registerItem(Item::Builder item) {
	item.name = name_ + "::" + item.name;
	items_.push_back(item);
	info << "  Adding item: " << item.name;
}

Iter<std::unique_ptr<ImageResource>> Mod::buildImages(SDL_Renderer *renderer) {
	return map(begin(images_), end(images_), [renderer, this](const std::string &id) {
		return std::make_unique<ImageResource>(renderer, path_, id);
	});
}

Iter<std::unique_ptr<Tile>> Mod::buildTiles(const ResourceManager &resources) {
	return map(begin(tiles_), end(tiles_), [&](const Tile::Builder &builder) {
		return std::make_unique<Tile>(resources, builder);
	});
}

Iter<std::unique_ptr<Item>> Mod::buildItems(const ResourceManager &resources) {
	return map(begin(items_), end(items_), [&](const Item::Builder &builder) {
		return std::make_unique<Item>(resources, builder);
	});
}

Iter<WorldGen::Factory> Mod::getWorldGens() {
	return map(begin(worldgens_), end(worldgens_), [](WorldGen::Factory &fact) {
		return fact;
	});
}

Iter<Entity::Factory> Mod::getEntities() {
	return map(begin(entities_), end(entities_), [](Entity::Factory &fact){
		return fact;
	});
}

}
