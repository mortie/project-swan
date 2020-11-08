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
		bool is_solid = true;
		uint8_t light_level = 0;
		std::optional<std::string> dropped_item = std::nullopt;
	};

	Tile(const ResourceManager &resources, const Builder &builder):
		name_(builder.name), image_(resources.getImage(builder.image)),
		is_solid_(builder.is_solid), light_level_(builder.light_level),
		dropped_item_(builder.dropped_item) {}

	const std::string name_;
	const ImageResource &image_;
	const bool is_solid_;
	const uint8_t light_level_;
	const std::optional<std::string> dropped_item_;

	static std::unique_ptr<Tile> createInvalid(const ResourceManager &ctx);
	static std::unique_ptr<Tile> createAir(const ResourceManager &ctx);
	static ID INVALID_ID;
};

}
