#pragma once

#include <stdint.h>
#include <string>
#include <optional>
#include <memory>

#include "Item.h"
#include "Resource.h"

namespace Swan {

struct Tile {
public:
	using ID = uint16_t;

	struct Builder {
		std::string name;
		std::string image;
		bool isSolid = true;
		float lightLevel = 0;
		std::optional<std::string> droppedItem = std::nullopt;
	};

	Tile(const ResourceManager &resources, const Builder &builder):
		name(builder.name), image(resources.getImage(builder.image)),
		isSolid(builder.isSolid), lightLevel(builder.lightLevel),
		droppedItem(builder.droppedItem) {}

	const std::string name;
	const ImageResource &image;
	const bool isSolid;
	const float lightLevel;
	const std::optional<std::string> droppedItem;

	static std::unique_ptr<Tile> createInvalid(const ResourceManager &ctx);
	static std::unique_ptr<Tile> createAir(const ResourceManager &ctx);
	static ID INVALID_ID;
};

}
