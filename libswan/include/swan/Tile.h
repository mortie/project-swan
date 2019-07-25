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

	static sf::Image invalid_image;
	static Tile invalid_tile;
	static void initInvalid();

	bool is_solid_ = true;

	std::string path_;
	std::string name_;
	sf::Image image_;
};

}
