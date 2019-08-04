#include "Asset.h"

#include "common.h"

namespace Swan {

Asset Asset::INVALID_ASSET("");

bool Asset::load(const std::string &pfx) {
	if (!image_.loadFromFile(pfx + "/" + path_))
		return false;

	auto size = image_.getSize();
	tex_.create(size.x, size.y);
	tex_.update(image_);
	return true;
}

void Asset::initGlobal() {
	INVALID_ASSET.name_ = "@internal::invalid";
	INVALID_ASSET.image_.create(TILE_SIZE, TILE_SIZE, sf::Color(245, 66, 242));
	INVALID_ASSET.tex_.create(TILE_SIZE, TILE_SIZE);
	INVALID_ASSET.tex_.update(INVALID_ASSET.image_);
}

}
