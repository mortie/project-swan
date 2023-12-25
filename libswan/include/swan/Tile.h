#pragma once

#include <stdint.h>
#include <string>
#include <optional>

#include "common.h"

namespace Swan {

struct Tile {
	using ID = uint16_t;

	struct Builder {
		std::string name;
		std::string image;
		bool isSolid = true;
		bool isOpaque = isSolid;
		float lightLevel = 0;
		std::optional<std::string> droppedItem = std::nullopt;

		void (*onSpawn)(const Context &ctx, TilePos pos) = nullptr;
	};

	const ID id;
	const std::string name;
	const bool isSolid;
	const bool isOpaque;
	const float lightLevel;
	const std::optional<std::string> droppedItem;

	void (*const onSpawn)(const Context &ctx, TilePos pos);

	Tile(ID id, std::string name, const Builder &builder):
		id(id), name(name),
		isSolid(builder.isSolid), isOpaque(builder.isOpaque),
		lightLevel(builder.lightLevel),
		droppedItem(builder.droppedItem), onSpawn(builder.onSpawn) {}
};

}
