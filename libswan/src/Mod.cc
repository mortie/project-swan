#include "Mod.h"

#include <stdio.h>
#include <algorithm>

#include "util.h"

namespace Swan {

void Mod::init(const std::string &name) {
	name_ = name;
	inited_ = true;

	fprintf(stderr, "Mod initing: %s\n", name_.c_str());
}

void Mod::registerImage(const std::string &name, const std::string &path, int frame_height) {
	images_.push_back(std::make_unique<ImageResource>(
		renderer_, name_ + "::" + name, path_ + "/assets/" + path, frame_height));
	fprintf(stderr, "Adding image: %s\n", (name_ + "::" + name).c_str());
}

void Mod::registerTile(Tile::Builder tile) {
	tile.name = name_ + "::" + tile.name;
	tiles_.push_back(tile);
	fprintf(stderr, "Adding tile: %s\n", tile.name.c_str());
}

void Mod::registerItem(Item::Builder item) {
	item.name = name_ + "::" + item.name;
	items_.push_back(item);
	fprintf(stderr, "Adding item: %s\n", item.name.c_str());
}

void Mod::registerWorldGen(const std::string &name, std::unique_ptr<WorldGen::Factory> gen) {
	gen->name_ = name_ + "::" + name;
	fprintf(stderr, "Adding world gen: %s\n", gen->name_.c_str());
	worldgens_.push_back(std::move(gen));
}

void Mod::registerEntity(const std::string &name, std::unique_ptr<Entity::Factory> ent) {
	ent->name_ = name_ + "::" + name;
	fprintf(stderr, "Adding entity: %s\n",ent->name_.c_str());
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
