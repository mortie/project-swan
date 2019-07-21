#include "Mod.h"

#include <stdio.h>

namespace Swan {

void Mod::init(const std::string &name) {
	name_ = name;
	inited_ = true;

	fprintf(stderr, "Mod initing: %s\n", name_.c_str());
}

void Mod::registerTile(const std::string &name, const std::string &asset, const Tile::Opts &opts) {
	tiles_.push_back(Tile());
	Tile &t = tiles_.back();
	t.name_ = name_ + "::" + name;
	t.opts_ = opts;
	fprintf(stderr, "Adding tile: %s\n", t.name_.c_str());

	std::string asset_path = path_ + "/" + asset;
	if (!t.image_.loadFromFile(asset_path)) {
		fprintf(stderr, "Tile %s: Failed to load image %s\n", t.name_.c_str(), asset_path.c_str());
	}
}

}
