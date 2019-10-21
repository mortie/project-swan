#pragma once

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace Swan {

class Asset {
public:
	Asset(const std::string &path): path_(path) {}

	bool load(const std::string &pfx);

	std::string name_;
	std::string path_;
	sf::Image image_;
	sf::Texture tex_;

	static Asset INVALID_ASSET;
	static void initGlobal();
};

}
