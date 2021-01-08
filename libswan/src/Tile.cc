#include "Tile.h"

#include "common.h"
#include <Game.h>

namespace Swan {

Tile::ID Tile::INVALID_ID = 0;

std::unique_ptr<Tile> Tile::createInvalid(const ResourceManager &resources) {
	return std::make_unique<Tile>(resources, Builder{
		.name = "@::invalid",
		.image = "@::invalid",
	});
}

std::unique_ptr<Tile> Tile::createAir(const ResourceManager &resources) {
	return std::make_unique<Tile>(resources, Builder{
		.name = "@::air",
		.image = "@::air",
		.isSolid = false,
	});
}

}
