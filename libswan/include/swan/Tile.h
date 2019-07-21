#pragma once

#include <stdint.h>
#include <string>
#include <SFML/Graphics/Image.hpp>

namespace Swan {

class Tile {
public:
	using ID = uint16_t;

	struct Opts {
		bool transparent_ = false;

		Opts &transparent() { transparent_ = true; return *this; }
	};

	std::string name_;
	sf::Image image_;
	Opts opts_;
};

}
