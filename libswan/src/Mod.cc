#include "Mod.h"

#include <stdio.h>
#include <algorithm>

#include "util.h"
#include "log.h"

namespace Swan {

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

Iter<std::unique_ptr<ImageResource>> ModWrapper::buildImages(SDL_Renderer *renderer) {
	return map(begin(mod_->images_), end(mod_->images_),
			[renderer, this](const std::string &id) {
		return std::make_unique<ImageResource>(renderer, path_, id);
	});
}

Iter<std::unique_ptr<Tile>> ModWrapper::buildTiles(const ResourceManager &resources) {
	return map(begin(mod_->tiles_), end(mod_->tiles_),
			[&](const Tile::Builder &builder) {
		return std::make_unique<Tile>(resources, builder);
	});
}

Iter<std::unique_ptr<Item>> ModWrapper::buildItems(const ResourceManager &resources) {
	return map(begin(mod_->items_), end(mod_->items_),
			[&](const Item::Builder &builder) {
		return std::make_unique<Item>(resources, builder);
	});
}

Iter<WorldGen::Factory> ModWrapper::getWorldGens() {
	return map(begin(mod_->worldgens_), end(mod_->worldgens_),
			[](WorldGen::Factory &fact) {
		return fact;
	});
}

Iter<EntityCollection::Factory> ModWrapper::getEntities() {
	return map(begin(mod_->entities_), end(mod_->entities_),
			[](EntityCollection::Factory &fact){
		return fact;
	});
}

}
