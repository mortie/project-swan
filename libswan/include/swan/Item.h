#pragma once

#include <string>

#include "Tile.h"

namespace Swan {

struct Item {
public:
	struct Builder {
		std::string name;
		std::string image;
		int maxStack = 64;
		std::optional<std::string> tile;
	};

	const Tile::ID id;
	const std::string name;
	const int maxStack;
	const Tile *tile;

	Item(Tile::ID id, std::string name, const Builder &builder):
		id(id), name(name), maxStack(builder.maxStack), tile(nullptr)
	{}
};

}
