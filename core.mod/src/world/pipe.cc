#include "pipe.h"
#include "tileentities/ItemPipeTileEntity.h"

#include <array>
#include <swan/swan.h>
#include <unordered_map>

namespace CoreMod {

struct PipeTileTrait: PipeConnectibleTileTrait {
	PipeTileTrait(std::string p): prefix(std::move(p))
	{}

	std::string prefix;
};

constexpr std::array<const char *, 16> DIRECTION_LUT = []() {
	// LEFT | RIGHT | UP | DOWN
	std::array<const char *, 16> lut;
	lut[0b0000] = "lone";
	lut[0b1000] = "l";
	lut[0b0100] = "r";
	lut[0b0010] = "u";
	lut[0b0001] = "d";
	lut[0b1100] = "h";
	lut[0b0011] = "v";
	lut[0b1010] = "lu";
	lut[0b1001] = "ld";
	lut[0b0110] = "ru";
	lut[0b0101] = "rd";
	lut[0b1110] = "lru";
	lut[0b1101] = "lrd";
	lut[0b1011] = "udl";
	lut[0b0111] = "udr";
	lut[0b1111] = "cross";
	return lut;
}();

static void onPipeUpdate(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto &tile = ctx.plane.tiles().get(pos);
	auto &prefix = dynamic_cast<PipeTileTrait *>(tile.more->traits.get())->prefix;

	auto check = [&](Swan::Direction dir) {
		auto checkPos = pos + dir;

		auto &tile = ctx.plane.tiles().get(checkPos);
		auto *connectible = dynamic_cast<PipeConnectibleTileTrait *>(
			tile.more->traits.get());
		if (
				connectible &&
				connectible->pipeConnectDirections.has(dir.opposite())) {
			return true;
		}

		auto ref = ctx.plane.entities().getTileEntity(checkPos);
		if (!ref) {
			return false;
		}

		return !!ref.trait<Swan::InventoryTrait>();
	};

	int key =
		(check(Swan::Direction::LEFT) << 3) |
			(check(Swan::Direction::RIGHT) << 2) |
			(check(Swan::Direction::UP) << 1) |
			(check(Swan::Direction::DOWN) << 0);

	std::string name = Swan::cat(prefix, "::", DIRECTION_LUT[key]);

	if (name != tile.name) {
		ctx.plane.tiles().set(pos, name);
	}
}

void registerGlassPipe(Swan::Mod &mod)
{
	auto traits = std::make_shared<PipeTileTrait>("core::glass-pipe");
	auto openPipeTraits = std::make_shared<PipeTileTrait>("core::glass-pipe");

	mod.registerEntity<ItemPipeTileEntity>("tile::item-pipe");

	mod.registerTile({
		.name = "glass-pipe",
		.image = "core::tiles/glass-pipe::cross",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.stepSound = "core::step/glass",
		.breakSound = "core::break/glass",
		.droppedItem = "core::glass-pipe",
		.onTileUpdate = onPipeUpdate,
		.traits = traits,
	});

	mod.registerTile({
		.name = "glass-pipe::cross",
		.image = "core::tiles/glass-pipe::cross",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.stepSound = "core::step/glass",
		.breakSound = "core::break/glass",
		.droppedItem = "core::glass-pipe",
		.tileEntity = "core::tile::item-pipe",
		.onTileUpdate = onPipeUpdate,
		.traits = traits,
	});

	mod.registerTile({
		.name = "glass-pipe::lone",
		.image = "core::tiles/glass-pipe::lone",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.stepSound = "core::step/glass",
		.breakSound = "core::break/glass",
		.droppedItem = "core::glass-pipe",
		.tileEntity = "core::tile::item-pipe",
		.onTileUpdate = onPipeUpdate,
		.traits = traits,
	});

	for (auto variant: {"lu", "ld", "ru", "rd"}) {
		mod.registerTile({
			.name = Swan::cat("glass-pipe::", variant),
			.image = Swan::cat("core::tiles/glass-pipe::", variant),
			.isSolid = false,
			.breakableBy = Swan::Tool::HAND,
			.stepSound = "core::step/glass",
			.breakSound = "core::break/glass",
			.droppedItem = "core::glass-pipe",
			.tileEntity = "core::tile::item-pipe",
			.onTileUpdate = onPipeUpdate,
			.traits = traits,
		});
	}

	for (auto variant: {"h", "v"}) {
		mod.registerTile({
			.name = Swan::cat("glass-pipe::", variant),
			.image = Swan::cat("core::tiles/glass-pipe::", variant),
			.isSolid = false,
			.breakableBy = Swan::Tool::HAND,
			.stepSound = "core::step/glass",
			.breakSound = "core::break/glass",
			.droppedItem = "core::glass-pipe",
			.tileEntity = "core::tile::item-pipe",
			.onTileUpdate = onPipeUpdate,
			.traits = traits,
		});
	}

	for (auto variant: {"l", "r", "u", "d"}) {
		mod.registerTile({
			.name = Swan::cat("glass-pipe::", variant),
			.image = Swan::cat("core::tiles/glass-pipe::", variant),
			.isSolid = false,
			.breakableBy = Swan::Tool::HAND,
			.stepSound = "core::step/glass",
			.breakSound = "core::break/glass",
			.droppedItem = "core::glass-pipe",
			.tileEntity = "core::tile::item-pipe",
			.onTileUpdate = onPipeUpdate,
			.traits = traits,
		});
	}

	for (auto variant: {"lru", "lrd", "udl", "udr"}) {
		mod.registerTile({
			.name = Swan::cat("glass-pipe::", variant),
			.image = Swan::cat("core::tiles/glass-pipe::", variant),
			.isSolid = false,
			.breakableBy = Swan::Tool::HAND,
			.stepSound = "core::step/glass",
			.breakSound = "core::break/glass",
			.droppedItem = "core::glass-pipe",
			.tileEntity = "core::tile::item-pipe",
			.onTileUpdate = onPipeUpdate,
			.traits = traits,
		});
	}
}

}
