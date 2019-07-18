#pragma once

#include <stdint.h>

namespace Swan {

class Tile {
public:
	using TileID = uint16_t;

	Tile(std::string name);
};

}
