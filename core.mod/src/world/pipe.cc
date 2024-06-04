#include "pipe.h"
#include "tileentities/ItemPipeTileEntity.h"

#include <array>
#include <swan/swan.h>
#include <unordered_map>

namespace CoreMod {

struct PipeTileTrait: Swan::Tile::Traits {
	PipeTileTrait(std::string p): prefix(std::move(p))
	{}

	std::string prefix;
};

constexpr std::array<const char *, 16> DIRECTION_LUT = []() {
	// LEFT | RIGHT | UP | DOWN
	std::array<const char *, 16> lut;
	lut[0b0000] = "lone";
	lut[0b1000] = "open::l";
	lut[0b0100] = "open::r";
	lut[0b0010] = "open::u";
	lut[0b0001] = "open::d";
	lut[0b1100] = "line::h";
	lut[0b0011] = "line::v";
	lut[0b1010] = "elbow::lu";
	lut[0b1001] = "elbow::ld";
	lut[0b0110] = "elbow::ru";
	lut[0b0101] = "elbow::rd";
	lut[0b1110] = "tee::lru";
	lut[0b1101] = "tee::lrd";
	lut[0b1011] = "tee::udl";
	lut[0b0111] = "tee::udr";
	lut[0b1111] = "cross";
	return lut;
}();

static void onPipeUpdate(const Swan::Context &ctx, Swan::TilePos pos)
{
	auto &tile = ctx.plane.getTile(pos);
	auto &prefix = dynamic_cast<PipeTileTrait *>(tile.traits.get())->prefix;

	auto check = [&](int x, int y) {
		auto ref = ctx.plane.getTileEntity(pos + Swan::TilePos{x, y});
		if (!ref) {
			return false;
		}

		return !!ref.trait<Swan::InventoryTrait>();
	};

	int key =
		(check(-1, 0) << 3) | // left
			(check(1, 0) << 2) | // right
			(check(0, -1) << 1) | // up
			(check(0, 1) << 0); // down

	std::string name = Swan::cat(prefix, "::", DIRECTION_LUT[key]);

	if (name != tile.name) {
		ctx.plane.setTile(pos, name);
	}
}

void registerGlassPipe(Swan::Mod &mod)
{
	auto traits = std::make_shared<PipeTileTrait>("core::glass-pipe");
	auto openPipeTraits = std::make_shared<PipeTileTrait>("core::glass-pipe");

	mod.registerEntity<ItemPipeTileEntity>("tile::input-pipe");

	mod.registerTile({
		.name = "glass-pipe",
		.image = "core::tiles/glass-pipe/cross",
		.isOpaque = false,
		.droppedItem = "core::glass-pipe",
		.onTileUpdate = onPipeUpdate,
		.traits = traits,
	});

	mod.registerTile({
		.name = "glass-pipe::cross",
		.image = "core::tiles/glass-pipe/cross",
		.isOpaque = false,
		.droppedItem = "core::glass-pipe",
		.onTileUpdate = onPipeUpdate,
		.traits = traits,
	});

	mod.registerTile({
		.name = "glass-pipe::lone",
		.image = "core::tiles/glass-pipe/lone",
		.isOpaque = false,
		.droppedItem = "core::glass-pipe",
		.onTileUpdate = onPipeUpdate,
		.tileEntity = "core::tile::input-pipe",
		.traits = traits,
	});

	for (auto variant: {"lu", "ld", "ru", "rd"}) {
		mod.registerTile({
			.name = Swan::cat("glass-pipe::elbow::", variant),
			.image = Swan::cat("core::tiles/glass-pipe/elbow::", variant),
			.isOpaque = false,
			.droppedItem = "core::glass-pipe",
			.onTileUpdate = onPipeUpdate,
			.traits = traits,
		});
	}

	for (auto variant: {"h", "v"}) {
		mod.registerTile({
			.name = Swan::cat("glass-pipe::line::", variant),
			.image = Swan::cat("core::tiles/glass-pipe/line::", variant),
			.isOpaque = false,
			.droppedItem = "core::glass-pipe",
			.onTileUpdate = onPipeUpdate,
			.traits = traits,
		});
	}

	for (auto variant: {"l", "r", "u", "d"}) {
		mod.registerTile({
			.name = Swan::cat("glass-pipe::open::", variant),
			.image = Swan::cat("core::tiles/glass-pipe/open::", variant),
			.isOpaque = false,
			.droppedItem = "core::glass-pipe",
			.onTileUpdate = onPipeUpdate,
			.tileEntity = "core::tile::input-pipe",
			.traits = traits,
		});
	}

	for (auto variant: {"lru", "lrd", "udl", "udr"}) {
		mod.registerTile({
			.name = Swan::cat("glass-pipe::tee::", variant),
			.image = Swan::cat("core::tiles/glass-pipe/tee::", variant),
			.isOpaque = false,
			.droppedItem = "core::glass-pipe",
			.onTileUpdate = onPipeUpdate,
			.traits = traits,
		});
	}
}

}
