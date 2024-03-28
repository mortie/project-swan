#include "pipe.h"

namespace CoreMod {

struct PipeTileTrait: PipeConnectableTileTrait {
	PipeTileTrait(std::string p): prefix(std::move(p))
	{}
	std::string prefix;
};

static void onPipeUpdate(const Swan::Context &ctx, Swan:: TilePos pos)
{
	auto &tile = ctx.plane.getTile(pos);
	auto &prefix = dynamic_cast<PipeTileTrait *>(tile.traits.get())->prefix;

	auto check = [&](int x, int y) {
		auto &t = ctx.plane.getTile(pos + Swan::TilePos{x, y});
		return !!dynamic_cast<PipeConnectableTileTrait *>(t.traits.get());
	};

	int key =
		(check(-1, 0) << 3) | // left
			(check(1, 0) << 2) | // right
			(check(0, -1) << 1) | // up
			(check(0, 1) << 0); // down

	constexpr const char *lut[] = {
		[0b0000] = "lone",
		[0b1000] = "open::l",
		[0b0100] = "open::r",
		[0b0010] = "open::u",
		[0b0001] = "open::d",
		[0b1100] = "line::h",
		[0b0011] = "line::v",
		[0b1010] = "elbow::lu",
		[0b1001] = "elbow::ld",
		[0b0110] = "elbow::ru",
		[0b0101] = "elbow::rd",
		[0b1110] = "tee::lru",
		[0b1101] = "tee::lrd",
		[0b1011] = "tee::udl",
		[0b0111] = "tee::udr",
		[0b1111] = "cross",
	};

	std::string name = Swan::cat(prefix, "::", lut[key]);

	if (name != tile.name) {
		Swan::info << "Update: " << tile.name << " -> " << name;
		ctx.plane.setTile(pos, name);
	}
}

void registerGlassPipe(Swan::Mod &mod)
{
	auto traits = std::make_shared<PipeTileTrait>("core::glass-pipe");

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
