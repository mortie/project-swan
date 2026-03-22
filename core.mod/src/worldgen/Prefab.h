#pragma once

#include <swan/swan.h>
#include <initializer_list>
#include <memory>
#include <span>

namespace CoreMod {

struct Prefab {
	struct Mapping {
		char symbol;
		const char *name;
	};

	using Map = std::initializer_list<Mapping>;
	using Data = std::initializer_list<const char *>;

	int width;
	int height;
	std::unique_ptr<Swan::Tile::ID[]> tiles;

	Prefab(Swan::World &world, Map map, Data data);
	Prefab(Prefab &&) = default;
};

}
