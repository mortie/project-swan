#pragma once

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace Swan {

class Asset {
public:
	Asset(std::string path): path_(path) {}

	bool load(std::string pfx) {
		if (!img_.loadFromFile(pfx + "/" + path_))
			return false;

		auto size = img_.getSize();
		tex_.create(size.x, size.y);
		tex_.update(img_);
		return true;
	}

	std::string name_;
	std::string path_;
	sf::Image img_;
	sf::Texture tex_;
};

}
