#pragma once

#include <stdint.h>
#include <string>

namespace Swan {

class Tile {
public:
	using TileID = uint16_t;

	std::string name_;
};

}
