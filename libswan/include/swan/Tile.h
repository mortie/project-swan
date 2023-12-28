#pragma once

#include <memory>
#include <stdint.h>
#include <string>
#include <optional>

#include "common.h"

namespace Swan {

struct Tile {
	using ID = uint16_t;

	struct Traits {
		virtual ~Traits() = default;
	};

	struct Builder {
		std::string name;
		std::string image;
		bool isSolid = true;
		bool isOpaque = isSolid;
		float lightLevel = 0;
		std::optional<std::string> droppedItem = std::nullopt;

		void (*onSpawn)(const Context &ctx, TilePos pos) = nullptr;
		void (*onBreak)(const Context &ctx, TilePos pos) = nullptr;
		void (*onTileUpdate)(const Context &ctx, TilePos pos) = nullptr;

		std::shared_ptr<Traits> traits = nullptr;
	};

	const ID id;
	const std::string name;
	const bool isSolid;
	const bool isOpaque;
	const float lightLevel;
	const std::optional<std::string> droppedItem;

	void(*const onSpawn)(const Context & ctx, TilePos pos);
	void(*const onBreak)(const Context & ctx, TilePos pos);
	void (*onTileUpdate)(const Context &ctx, TilePos pos) = nullptr;

	std::shared_ptr<Traits> traits;

	Tile(ID id, std::string name, const Builder &builder):
		id(id), name(name),
		isSolid(builder.isSolid), isOpaque(builder.isOpaque),
		lightLevel(builder.lightLevel),
		droppedItem(builder.droppedItem),
		onSpawn(builder.onSpawn), onBreak(builder.onBreak), onTileUpdate(builder.onTileUpdate),
		traits(builder.traits)
	{}
};

}
