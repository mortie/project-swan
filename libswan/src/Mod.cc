#include "Mod.h"

#include <stdio.h>

namespace Swan {

void Mod::init(const std::string &name) {
	name_ = name;
	inited_ = true;

	fprintf(stderr, "Mod initing: %s\n", name_.c_str());
}

void Mod::registerImage(const std::string &name, const std::string &path, int frame_height) {
	images_[name] = std::make_unique<ImageResource>(
		renderer_, name_ = "::" + name, path, frame_height);
	fprintf(stderr, "Adding image: %s\n", images_[name]->name_.c_str());
}

void Mod::registerTile(const Tile::Builder &tile) {
	tiles_[tile.name] = tile;
	fprintf(stderr, "Adding tile: %s::%s\n", name_.c_str(), tile.name.c_str());
}

void Mod::registerItem(const Item::Builder &item) {
	items_[item.name] = item;
	fprintf(stderr, "Adding item: %s::%s\n", name_.c_str(), item.name.c_str());
}

void Mod::registerWorldGen(const std::string &name, std::unique_ptr<WorldGen::Factory> gen) {
	gen->name_ = name_ + "::" + name;
	fprintf(stderr, "Adding world gen: %s\n", gen->name_.c_str());
	worldgens_[name] = std::move(gen);
}

void Mod::registerEntity(const std::string &name, std::unique_ptr<Entity::Factory> ent) {
	ent->name_ = name_ + "::" + name;
	fprintf(stderr, "Adding entity: %s\n",ent->name_.c_str());
	entities_[name] = std::move(ent);
}

}
