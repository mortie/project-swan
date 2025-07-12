#include "aqueduct.h"

namespace CoreMod {

struct AqueductTileTrait: Swan::Tile::Traits {
	AqueductTileTrait(int dir, int level):
		direction(dir), level(level)
	{}

	int direction;
	int level;
};

static bool spawnAqueduct(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto left = dynamic_cast<AqueductTileTrait *>(
		ctx.plane.tiles().get(pos.add(-1, 0)).traits.get());
	auto right = dynamic_cast<AqueductTileTrait *>(
		ctx.plane.tiles().get(pos.add(1, 0)).traits.get());

	Swan::CowStr variant;

	// Going the same direction as the thing on the left
	if (left && left->direction > 0) {
		variant = left->level > 0
			? Swan::cat("core::aqueduct::right::", left->level - 1)
			: "core::aqueduct::left::0";
	}

	// Going the same direction as the thing on the right
	else if (right && right->direction < 0) {
		variant = right->level > 0
			? Swan::cat("core::aqueduct::left::", right->level - 1)
			: "core::aqueduct::right::0";
	}

	// Going the opposite direction of the thing on the left
	else if (left && left->direction < 0) {
		auto left2 = dynamic_cast<AqueductTileTrait *>(
			ctx.plane.tiles().get(pos.add(-2, 0)).traits.get());
		if (left2) {
			variant = Swan::cat("core::aqueduct::right::", left->level);
		} else {
			ctx.plane.tiles().set(
				pos.add(-1, 0),
				Swan::cat("core::aqueduct::right::", left->level));
			return spawnAqueduct(ctx, pos);
		}
	}

	// Going the opposite direction of the thing on the right
	else if (right && right->direction > 0) {
		auto right2 = dynamic_cast<AqueductTileTrait *>(
			ctx.plane.tiles().get(pos.add(2, 0)).traits.get());
		if (right2) {
			variant = Swan::cat("core::aqueduct::left::", right->level);
		} else {
			ctx.plane.tiles().set(
				pos.add(-1, 0),
				Swan::cat("core::aqueduct::left::", right->level));
			return spawnAqueduct(ctx, pos);
		}
	}

	// Default
	else {
		ctx.plane.tiles().set(pos, "core::aqueduct::left::7");
		return true;
	}

	Swan::info << "Setting: " << variant.str();
	ctx.plane.tiles().set(pos, variant.str());

	return true;
}

void registerAqueduct(Swan::Mod &mod)
{
	mod.registerTile({
		.name = "aqueduct",
		.image = "core::tiles/aqueduct::right@7",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::aqueduct",
		.onSpawn = spawnAqueduct,
	});

	for (auto direction: {"left", "right"}) {
		int dir = std::string_view(direction) == "left" ? -1 : 1;
		for (int level = 0; level < 8; ++level) {
			mod.registerTile({
				.name = Swan::cat("aqueduct::", direction, "::", level),
				.image = Swan::cat("core::tiles/aqueduct::", direction, "@", level),
				.isSolid = false,
				.breakableBy = Swan::Tool::HAND,
				.droppedItem = "core::aqueduct",
				.traits = std::make_shared<AqueductTileTrait>(dir, level),
			});
		}
	}
}

}
