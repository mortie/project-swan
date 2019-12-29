#include "Mod.h"

#include <stdio.h>
#include <algorithm>

#include "util.h"
#include "log.h"

namespace Swan {

void Mod::init(const std::string &name) {
	name_ = name;
	inited_ = true;

	info << "Mod initing: " << name_;
}

void Mod::registerImage(const std::string &name, const std::string &path, int frame_height) {
	images_.push_back(std::make_unique<ImageResource>(
		renderer_, name_ + "::" + name, path_ + "/assets/" + path, frame_height));
	info << "Adding image: " << name_ << "::" << name;
}

void Mod::registerTile(Tile::Builder tile) {
	tile.name = name_ + "::" + tile.name;
	tiles_.push_back(tile);
	info << "Adding tile: " << tile.name;
}

void Mod::registerItem(Item::Builder item) {
	item.name = name_ + "::" + item.name;
	items_.push_back(item);
	info << "Adding item: " << item.name;
}

void Mod::registerWorldGen(const std::string &name, std::unique_ptr<WorldGen::Factory> gen) {
	gen->name_ = name_ + "::" + name;
	info << "Adding world gen: " << gen->name_;
	worldgens_.push_back(std::move(gen));
}

void Mod::registerEntity(const std::string &name, std::unique_ptr<Entity::Factory> ent) {
	ent->name_ = name_ + "::" + name;
	info << "Adding entity: " << ent->name_;
	entities_.push_back(std::move(ent));
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

Iter<WorldGen::Factory *> Mod::getWorldGens() {
	return map(begin(worldgens_), end(worldgens_), [](const std::unique_ptr<WorldGen::Factory> &gen) {
		return gen.get();
	});
}

Iter<Entity::Factory *> Mod::getEntities() {
	return map(begin(entities_), end(entities_), [](const std::unique_ptr<Entity::Factory> &ent){
		return ent.get();
	});
}

}
