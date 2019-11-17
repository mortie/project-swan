#pragma once

#include <stdint.h>
#include <string>
#include <optional>
#include <memory>

#include "Item.h"
#include "Resource.h"

namespace Swan {

class Tile {
public:
	using ID = uint16_t;

	struct Builder {
		std::string name;
		std::string image;
		bool is_solid = true;
		std::optional<std::string> dropped_item = std::nullopt;
	};

	Tile(const ImageResource &image, const std::string &mod, const Builder &builder):
		name_(mod+"::"+builder.name), image_(image),
		is_solid_(builder.is_solid), dropped_item_(builder.dropped_item) {}

	const std::string name_;
	const ImageResource &image_;
	const bool is_solid_;
	const std::optional<std::string> dropped_item_;

	static std::unique_ptr<Tile> createInvalid(Context &ctx);
	static ID INVALID_ID;
};

}
