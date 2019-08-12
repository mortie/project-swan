#include "Tile.h"
#include "common.h"

namespace Swan {

Tile Tile::INVALID_TILE;
Tile::ID Tile::INVALID_ID = 0;

static void initInvalid(Tile *t) {
	t->name = "@internal::invalid";
	t->image.reset(new sf::Image());
	t->image->create(TILE_SIZE, TILE_SIZE, sf::Color(245, 66, 242));
	t->is_solid = false;
}

Tile *Tile::createInvalid() {
	Tile *t = new Tile();
	initInvalid(t);
	return t;
}

void Tile::initGlobal() {
	initInvalid(&INVALID_TILE);
}

}
