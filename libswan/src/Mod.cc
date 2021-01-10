#include "Mod.h"

#include <stdio.h>
#include <algorithm>

#include "util.h"
#include "log.h"

namespace Swan {

void Mod::registerTile(Tile::Builder tile) {
	tiles_.push_back(tile);
	info << "  Adding tile: " << name_ << "::" << tile.name;
}

void Mod::registerItem(Item::Builder item) {
	items_.push_back(item);
	info << "  Adding item: " << name_ << "::" << item.name;
}

void Mod::registerSprite(std::string path) {
	sprites_.push_back(path);
	info << "Adding sprite: " << name_ << "::" << path;
}

}
