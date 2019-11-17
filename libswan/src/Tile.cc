#include "Tile.h"

#include "common.h"
#include <Game.h>

namespace Swan {

Tile::ID Tile::INVALID_ID = 0;

std::unique_ptr<Tile> Tile::createInvalid(Context &ctx) {
	return std::make_unique<Tile>(*ctx.game.invalid_image_, "@internal", Builder{
		.name = "invalid",
		.image = "invalid",
	});
}

}
