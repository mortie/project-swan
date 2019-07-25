#pragma once

#include <stdint.h>
#include <string>
#include <SFML/Graphics/Image.hpp>

namespace Swan {

class Tile {
public:
	using ID = uint16_t;

	Tile(std::string path): path_(path) {}

	Tile *solid(bool b) { is_solid_ = b; return this; }

	bool is_solid_ = true;

	std::string path_;
	std::string name_;
	sf::Image image_;

	static sf::Image INVALID_IMAGE;
	static Tile INVALID_TILE;
	static void initInvalid();
};

}
