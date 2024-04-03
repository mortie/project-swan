#pragma once

#include <memory>
#include <stdint.h>
#include <string>
#include <optional>

#include "common.h"
#include "assets.h"

namespace Swan {

struct Item;

struct Tile {
	using ID = uint16_t;

	struct Traits {
		virtual ~Traits() = default;
	};

	struct Builder {
		std::string name;
		std::string image = "@::invalid";
		bool isSolid = true;
		bool isOpaque = isSolid;
		float lightLevel = 0;
		std::optional<std::string> stepSound = std::nullopt;
		std::optional<std::string> breakSound = std::nullopt;
		std::optional<std::string> droppedItem = std::nullopt;

		void (*onSpawn)(const Context &ctx, TilePos pos) = nullptr;
		void (*onBreak)(const Context &ctx, TilePos pos) = nullptr;
		void (*onTileUpdate)(const Context &ctx, TilePos pos) = nullptr;

		std::optional<std::string> tileEntity = std::nullopt;

		std::shared_ptr<Traits> traits = nullptr;
	};

	ID id;
	std::string name;
	bool isSolid;
	bool isOpaque;
	float lightLevel;
	SoundAsset *stepSounds[2] = {nullptr, nullptr};
	SoundAsset *breakSound = nullptr;
	Item *droppedItem = nullptr;

	void (*onSpawn)(const Context &ctx, TilePos pos);
	void (*onBreak)(const Context &ctx, TilePos pos);
	void (*onTileUpdate)(const Context &ctx, TilePos pos);

	std::optional<std::string> tileEntity;

	std::shared_ptr<Traits> traits;

	Tile() = default;
	Tile(ID id, std::string name, const Builder &builder):
		id(id), name(name),
		isSolid(builder.isSolid), isOpaque(builder.isOpaque),
		lightLevel(builder.lightLevel),
		onSpawn(builder.onSpawn), onBreak(builder.onBreak),
		onTileUpdate(builder.onTileUpdate),
		tileEntity(builder.tileEntity), traits(builder.traits)
	{}
};

}
