#pragma once

#include <memory>
#include <string>
#include <SFML/Graphics/Image.hpp>

namespace Swan {

struct Item {
	std::unique_ptr<sf::Image> image;

	std::string name = "";

	static Item INVALID_ITEM;
	static Item *createInvalid();
	static void initGlobal();
};

}
