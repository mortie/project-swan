#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <SFML/Graphics/Image.hpp>

namespace Swan {

struct Tile {
public:
	using ID = uint16_t;

	std::unique_ptr<sf::Image> image;
	bool is_solid = true;
	std::string dropped_item = "";

	std::string name = "";

	static Tile INVALID_TILE;
	static ID INVALID_ID;
	static Tile *createInvalid();
	static void initGlobal();
};

}
