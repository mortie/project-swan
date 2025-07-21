#pragma once

#include <bitset>
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
#include "swan/constants.h"

namespace Swan {

struct Item;
class ItemStack;

struct Tile {
	using ID = uint16_t;

	enum Flags: uint8_t {
		NONE = 0,
		IS_SOLID = 1 << 0,
		IS_OPAQUE = 1 << 1,
		IS_SUPPORT_V = 1 << 2,
		IS_SUPPORT_H = 1 << 3,
		IS_REPLACABLE = 1 << 4,
	};

	friend constexpr Tile::Flags operator|(Tile::Flags a, Tile::Flags b) {
		return Tile::Flags(int(a) | int(b));
	}

	friend constexpr bool operator&(Tile::Flags a, Tile::Flags b) {
		return int(a) & int(b);
	}

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

		std::shared_ptr<FluidCollision> fluidCollision = nullptr;
		std::shared_ptr<Traits> traits = nullptr;
	};

	struct More {
		More(const Builder &builder):
			onSpawn(builder.onSpawn),
			onBreak(builder.onBreak),
			onTileUpdate(builder.onTileUpdate),
			onActivate(builder.onActivate),
			onWorldTick(builder.onWorldTick),
			fluidCollision(builder.fluidCollision),
			traits(builder.traits),
			tileEntity(builder.tileEntity),
			lightLevel(builder.lightLevel),
			temperature(builder.temperature)
		{}

		bool (*onSpawn)(Ctx &ctx, TilePos pos);
		void (*onBreak)(Ctx &ctx, TilePos pos);
		void (*onTileUpdate)(Ctx &ctx, TilePos pos);
		void (*onActivate)(Ctx &ctx, TilePos pos, ActivateMeta meta);
		void (*onWorldTick)(Ctx &ctx, TilePos pos);

		std::shared_ptr<TileParticles> particles;
		std::shared_ptr<FluidCollision> fluidCollision;
		std::shared_ptr<Traits> traits;

		SoundAsset *stepSounds[2] = {nullptr, nullptr};
		SoundAsset *placeSound = nullptr;
		SoundAsset *breakSound = nullptr;
		Item *droppedItem = nullptr;
		ROString tileEntity;

		float lightLevel;
		float temperature;
	};

	ID id;
	Flags flags;
	ToolSet breakableBy;
	// 4 bytes padding
	ROString name;

	std::unique_ptr<More> more;

	Tile() = default;
	Tile(const Tile &) = delete;
	Tile(Tile &&) = default;

	Tile(ID id, std::string name, const Builder &builder):
		id(id),
		flags(
			(builder.isSolid ? IS_SOLID : NONE) |
			(builder.isOpaque ? IS_OPAQUE : NONE) |
			(builder.isSupportV ? IS_SUPPORT_V : NONE) |
			(builder.isSupportH ? IS_SUPPORT_H : NONE) |
			(builder.isReplacable ? IS_REPLACABLE : NONE)),
		breakableBy(builder.breakableBy),
		name(name),
		more(std::make_unique<More>(builder))
	{}

	bool isSolid() const { return flags & IS_SOLID; }
	bool isOpaque() const { return flags & IS_OPAQUE; }
	bool isSupportV() const { return flags & IS_SUPPORT_V; }
	bool isSupportH() const { return flags & IS_SUPPORT_H; }
	bool isReplacable() const { return flags & IS_REPLACABLE; }
};

}
