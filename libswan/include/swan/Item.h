#pragma once

#include <string>

#include "Resource.h"
#include "Tile.h"

namespace Swan {

class Item {
public:
	struct Builder {
		std::string name;
		std::string image;
		int maxStack = 64;
	};

	const Tile::ID id;
	const std::string name;
	const int maxStack;

	Item(Tile::ID id, std::string name, const Builder &builder):
		id(id), name(name), maxStack(builder.maxStack) {}
};

}
