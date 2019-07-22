#pragma once

#include <stdint.h>
#include <string>
#include <SFML/Graphics/Image.hpp>

namespace Swan {

class Tile {
public:
	using ID = uint16_t;

	struct Opts {
		bool solid_ = true;
		Opts &solid(bool b) { solid_ = b; return *this; }
	};

	std::string name_;
	sf::Image image_;
	Opts opts_;

	static sf::Image invalid_image;
	static Tile invalid_tile;
	static void initInvalid();
};

}
