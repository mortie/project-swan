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

namespace Swan {

struct Item;
class ItemStack;

struct Tile {
	using ID = uint16_t;

	using FluidMaskIndex = uint16_t;

	enum Flags: uint8_t {
		NONE = 0,
		IS_SOLID = 1 << 0,
		IS_OPAQUE = 1 << 1,
		IS_SUPPORT_V = 1 << 2,
		IS_SUPPORT_H = 1 << 3,
		IS_REPLACABLE = 1 << 4,
		IS_PLATFORM = 1 << 5,
		IS_FULL_SUPPORT_H = 1 << 6,
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
		std::optional<std::string> fluidMask = std::nullopt;
		bool isSolid = true;
		bool isOpaque = isSolid;
		bool isSupportV = isSolid;
		bool isSupportH = isSolid;
		bool isReplacable = false;
		bool isPlatform = isSolid;
		bool isFullSupportH = isSupportH;
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

		Builder clone() { return *this; }

		Builder &withName(std::string name);
		Builder &withImage(std::string image);
		Builder &withFluidMask(std::string fluidMask);
		Builder &withIsSolid(bool isSolid);
		Builder &withIsOpaque(bool isOpaque);
		Builder &withIsSupportV(bool isSupportV);
		Builder &withIsSupportH(bool isSupportH);
		Builder &withIsPlatform(bool isPlatform);
		Builder &withIsReplacable(bool isReplacable);
		Builder &withLightLevel(float lightLevel);
		Builder &withTemperature(float temperature);
		Builder &withBreakableBy(ToolSet breakableBy);

		Builder &withStepSound(std::string stepSound);
		Builder &withPlaceSound(std::string placeSound);
		Builder &withBreakSound(std::string breakSound);
		Builder &withDroppedItem(std::string droppedItem);
		Builder &withTileEntity(std::string tileEntity);

		Builder &withOnSpawn(bool (*onSpawn)(Ctx &, TilePos));
		Builder &withOnBreak(void (*onBreak)(Ctx &, TilePos));
		Builder &withOnTileUpdate(void (*onTileUpdate)(Ctx &, TilePos));
		Builder &withOnActivate(void (*onActivate)(Ctx &, TilePos, ActivateMeta));
		Builder &withOnWorldTick(void (*onWorldTick)(Ctx &, TilePos));

		Builder &withFluidCollision(std::shared_ptr<FluidCollision> fluidCollision);
		Builder &withTraits(std::shared_ptr<Traits> traits);
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

		Cygnet::RenderMask fluidMask;
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
			(builder.isReplacable ? IS_REPLACABLE : NONE) |
			(builder.isPlatform ? IS_PLATFORM : NONE) |
			(builder.isFullSupportH ? IS_FULL_SUPPORT_H : NONE)),
		breakableBy(builder.breakableBy),
		name(name),
		more(std::make_unique<More>(builder))
	{}

	bool isSolid() const { return flags & IS_SOLID; }
	bool isOpaque() const { return flags & IS_OPAQUE; }
	bool isSupportV() const { return flags & IS_SUPPORT_V; }
	bool isSupportH() const { return flags & IS_SUPPORT_H; }
	bool isReplacable() const { return flags & IS_REPLACABLE; }
	bool isPlatform() const { return flags & IS_PLATFORM; }
	bool isFullSupportH() const { return flags & IS_FULL_SUPPORT_H; }
};

}
