#include "Tile.h"
#include "common.h"

namespace Swan {

sf::Image Tile::INVALID_IMAGE;
Tile Tile::INVALID_TILE("");
Tile::ID Tile::INVALID_ID = 0;

static void initInvalid(Tile *t) {
	t->name_ = "@internal::invalid";
	t->image_ = Tile::INVALID_IMAGE;
	t->is_solid_ = false;
}

Tile *Tile::createInvalid() {
	Tile *t = new Tile("");
	initInvalid(t);
	return t;
}

void Tile::initGlobal() {
	INVALID_IMAGE.create(TILE_SIZE, TILE_SIZE, sf::Color(245, 66, 242));
	initInvalid(&INVALID_TILE);
}

}
