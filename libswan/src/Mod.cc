#include "Mod.h"

#include <stdio.h>

namespace Swan {

void Mod::init(const std::string &name) {
	name_ = name;
	inited_ = true;

	fprintf(stderr, "Mod initing: %s\n", name_.c_str());
}

void Mod::registerTile(const std::string &name, const Tile &tile) {
	tiles_.push_back(tile);
	Tile &t = tiles_.back();
	t.name_ = name_ + "::" + name;

	fprintf(stderr, "Mod %s added tile %s.\n", name_.c_str(), t.name_.c_str());
}

}
