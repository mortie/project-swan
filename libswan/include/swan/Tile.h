#pragma once

#include <stdint.h>
#include <string>
#include <SFML/Graphics/Image.hpp>

namespace Swan {

class Tile {
public:
	using TileID = uint16_t;

	std::string name_;
	sf::Image image_;
};

}
