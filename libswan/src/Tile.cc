#include "Tile.h"
#include "common.h"

namespace Swan {

sf::Image Tile::INVALID_IMAGE;
Tile Tile::INVALID_TILE("");

void Tile::initInvalid() {
	INVALID_IMAGE.create(TILE_SIZE, TILE_SIZE, sf::Color(245, 66, 242));
	INVALID_TILE.name_ = "INVALID";
	INVALID_TILE.image_ = INVALID_IMAGE;
	INVALID_TILE.is_solid_ = false;
}

}
