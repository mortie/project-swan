#pragma once

#include <stdint.h>
#include <string>
#include <optional>
#include <memory>

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

	const ID id;
	const std::string name;
	const bool isSolid;
	const float lightLevel;
	const std::optional<std::string> droppedItem;

	Tile(ID id, std::string name, const Builder &builder):
		id(id), name(name),
		isSolid(builder.isSolid), lightLevel(builder.lightLevel),
		droppedItem(builder.droppedItem) {}
};

}
