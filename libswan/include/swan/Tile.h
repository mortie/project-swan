#pragma once

#include <cstdint>
#include <memory>
#include <stdint.h>
#include <string>
#include <optional>

#include "common.h"
#include "assets.h"
#include "Tool.h"
#include "EntityCollection.h"
#include "cygnet/util.h"

namespace Swan {

struct Item;
class ItemStack;

struct Tile {
	using ID = uint16_t;

	struct Traits {
		virtual ~Traits() = default;
	};

	struct ActivateMeta {
		EntityRef activator;
		ItemStack &stack;
	};

	struct Builder {
		std::string name;
		std::string image = "@::invalid";
		bool isSolid = true;
		bool isOpaque = isSolid;
		bool isSupportV = isSolid;
		bool isSupportH = isSolid;
		bool isReplacable = false;
		float lightLevel = 0;
		float temperature = 0;
		ToolSet breakableBy = Tool::NONE;

		std::optional<std::string> stepSound = std::nullopt;
		std::optional<std::string> placeSound = std::nullopt;
		std::optional<std::string> breakSound = std::nullopt;
		std::optional<std::string> droppedItem = std::nullopt;
		std::optional<std::string> tileEntity = std::nullopt;

		bool (*onSpawn)(Ctx &ctx, TilePos pos) = nullptr;
		void (*onBreak)(Ctx &ctx, TilePos pos) = nullptr;
		void (*onTileUpdate)(Ctx &ctx, TilePos pos) = nullptr;
		void (*onActivate)(Ctx &ctx, TilePos pos, ActivateMeta meta) = nullptr;
		void (*onWorldTick)(Ctx &ctx, TilePos pos) = nullptr;

		std::shared_ptr<Traits> traits = nullptr;
	};

	ID id;
	std::string name;
	bool isSolid;
	bool isOpaque;
	bool isSupportV;
	bool isSupportH;
	bool isReplacable;
	float lightLevel;
	float temperature;
	ToolSet breakableBy;

	SoundAsset *stepSounds[2] = {nullptr, nullptr};
	SoundAsset *placeSound = nullptr;
	SoundAsset *breakSound = nullptr;
	Item *droppedItem = nullptr;
	std::optional<std::string> tileEntity;

	bool (*onSpawn)(Ctx &ctx, TilePos pos);
	void (*onBreak)(Ctx &ctx, TilePos pos);
	void (*onTileUpdate)(Ctx &ctx, TilePos pos);
	void (*onActivate)(Ctx &ctx, TilePos pos, ActivateMeta meta);
	void (*onWorldTick)(Ctx &ctx, TilePos pos);

	Cygnet::ByteColor particles[8][8];

	std::shared_ptr<Traits> traits;

	Tile() = default;
	Tile(ID id, std::string name, const Builder &builder):
		id(id), name(name),
		isSolid(builder.isSolid), isOpaque(builder.isOpaque),
		isSupportV(builder.isSupportV), isSupportH(builder.isSupportH),
		isReplacable(builder.isReplacable), lightLevel(builder.lightLevel),
		temperature(builder.temperature),
		breakableBy(builder.breakableBy), tileEntity(builder.tileEntity),
		onSpawn(builder.onSpawn), onBreak(builder.onBreak),
		onTileUpdate(builder.onTileUpdate), onActivate(builder.onActivate),
		onWorldTick(builder.onWorldTick),
		traits(builder.traits)
	{}
};

}
