#pragma once

#include "swan/Tile.h"
#include <stdexcept>
#include <swan/swan.h>
#include <initializer_list>

namespace CoreMod {

struct Prefab {
	struct Mapping {
		char symbol;
		const Swan::Tile::ID &id;
	};

	using Map = std::initializer_list<Mapping>;
	using Data = std::initializer_list<std::string_view>;

	Prefab(Prefab &) = delete;
	Prefab(Prefab &&) = delete;

	Prefab &operator=(Prefab &) = delete;
	Prefab &operator=(Prefab &&) = delete;

	int width;
	int height;
	std::vector<const Swan::Tile::ID *> tiles;

	Prefab(Map map, Data data);
};

inline Prefab::Prefab(Map map, Data data)
{
	auto lookup = [&](char ch) -> const Swan::Tile::ID * {
		if (ch == ' ') {
			return &Swan::World::AIR_TILE_ID;
		}

		for (const auto &mapping: map) {
			if (mapping.symbol == ch) {
				return &mapping.id;
			}
		}

		throw std::runtime_error("Unmapped character in prefab");
	};

	width = 0;
	height = data.size();
	for (std::string_view row: data) {
		int rowLen = int(row.length());
		if (rowLen > width) {
			width = rowLen;
		}
	}

	if (width == 0 || height == 0) {
		width = 0;
		height = 0;
		return;
	}

	tiles.resize(width * height, &Swan::World::AIR_TILE_ID);
	int y = 0;
	for (std::string_view row: data) {
		int rowLen = int(row.length());
		for (int x = 0; x < rowLen; ++x) {
			tiles[y * width + x] = lookup(row[x]);
		}
		y += 1;
	}
}

}
