#include "Mod.h"

#include <stdio.h>

namespace Swan {

void Mod::init(const std::string &name) {
	name_ = name;
	inited_ = true;

	fprintf(stderr, "Mod initing: %s\n", name_.c_str());
}

void Mod::registerTile(const std::string &name, Tile *tile) {
	tile->name_ = name_ + "::" + name;
	fprintf(stderr, "Adding tile: %s\n", tile->name_.c_str());

	std::string asset_path = path_ + "/" + tile->path_;
	if (!tile->image_.loadFromFile(asset_path)) {
		fprintf(stderr, "Tile %s: Failed to load image %s\n", tile->name_.c_str(), asset_path.c_str());
		tile->image_ = Tile::INVALID_IMAGE;
	}

	tiles_.push_back(std::shared_ptr<Tile>(tile));
}

void Mod::registerWorldGen(const std::string &name, WorldGen::Factory *gen) {
	gen->name_ = name_ + "::" + name;
	worldgens_.push_back(std::shared_ptr<WorldGen::Factory>(gen));
}

void Mod::registerEntity(const std::string &name, Entity::Factory *ent) {
	ent->name_ = name_ + "::" + name;
	entities_.push_back(std::shared_ptr<Entity::Factory>(ent));
}

void Mod::registerAsset(const std::string &name, Asset *asset) {
	asset->name_ = name_ + "::" + name;

	if (!asset->load(path_)) {
		fprintf(stderr, "Asset %s: Failed to load image %s", name.c_str(), (path_ + "/" + asset->path_).c_str());
		abort();
	}

	assets_.push_back(std::shared_ptr<Asset>(asset));
}

}
