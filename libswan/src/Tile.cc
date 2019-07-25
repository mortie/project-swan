#include "Tile.h"
#include "common.h"

namespace Swan {

sf::Image Tile::invalid_image;
Tile Tile::invalid_tile("");

void Tile::initInvalid() {
	invalid_image.create(TILE_SIZE, TILE_SIZE, sf::Color(245, 66, 242));
	invalid_tile.name_ = "INVALID";
	invalid_tile.image_ = invalid_image;
	invalid_tile.is_solid_ = false;
}

}
