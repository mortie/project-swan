#include "Item.h"

#include "Resource.h"
#include "Game.h"
#include "common.h"

namespace Swan {

std::unique_ptr<Item> Item::createInvalid(Context &ctx) {
	return std::make_unique<Item>(*ctx.game.invalid_image_, "@internal", Builder{
		.name = "invalid",
		.image = "invalid",
	});
}

}
