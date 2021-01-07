#pragma once

#include <stdint.h>
#include <string>
#include <optional>
#include <memory>

#include "Item.h"
#include "Resource.h"

namespace Swan {

// TODO: Switch to struct
class Tile {
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
		name_(builder.name), image_(resources.getImage(builder.image)),
		isSolid_(builder.isSolid), lightLevel_(builder.lightLevel),
		droppedItem_(builder.droppedItem) {}

	const std::string name_;
	const ImageResource &image_;
	const bool isSolid_;
	const float lightLevel_;
	const std::optional<std::string> droppedItem_;

	static std::unique_ptr<Tile> createInvalid(const ResourceManager &ctx);
	static std::unique_ptr<Tile> createAir(const ResourceManager &ctx);
	static ID INVALID_ID;
};

}
