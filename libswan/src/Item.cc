#include "Item.h"
#include "common.h"

namespace Swan {

Item Item::INVALID_ITEM;

static void initInvalid(Item *i) {
	i->name = "@internal::invalid";
	i->image.reset(new sf::Image());
	i->image->create(TILE_SIZE, TILE_SIZE, sf::Color(254, 66, 242));
}

Item *Item::createInvalid() {
	Item *i = new Item();
	initInvalid(i);
	return i;
}

void Item::initGlobal() {
	initInvalid(&INVALID_ITEM);
}

}
