#include "ladder.h"

#include "util.h"

namespace CoreMod {

struct RopeLadderTileTrait: LadderTileTrait {
	RopeLadderTileTrait(bool isAnchor, std::string d):
		isAnchor(isAnchor), direction(std::move(d))
	{}

	bool isAnchor;
	std::string direction;
};

static bool spawnRopeLadderAnchor(const Swan::Context &ctx, Swan::TilePos pos)
{
	if (ctx.plane.getTile(pos.add(-1, 0)).isSupportH) {
		ctx.plane.setTile(pos, "core::rope-ladder::anchor::left");
	}
	else if (ctx.plane.getTile(pos.add(1, 0)).isSupportH) {
		ctx.plane.setTile(pos, "core::rope-ladder::anchor::right");
	}
	else {
		return false;
	}

	Swan::TilePos below = pos.add(0, 1);
	while (true) {
		auto &tile = ctx.plane.getTile(below);
		if (!dynamic_cast<RopeLadderTileTrait *>(tile.traits.get())) {
			break;
		}

		ctx.plane.scheduleTileUpdate(below);
		below = below.add(0, 1);
	}

	return true;
}

static void cascadeRopeLadder(const Swan::Context &ctx, Swan::TilePos pos)
{
	auto &tile = ctx.plane.getTile(pos);
	auto *ropeLadderTrait = dynamic_cast<RopeLadderTileTrait *>(tile.traits.get());

	if (!ropeLadderTrait) {
		Swan::warn
			<< tile.name << " wants to cascade but doesn't "
			<< "implement RopeLadderTileTrait";
		return;
	}

	// Break if appropriate
	int remainingLength = 15;
	if (ropeLadderTrait->isAnchor) {
		Swan::TilePos adjacentPos;
		if (ropeLadderTrait->direction == "left") {
			adjacentPos = pos.add(-1, 0);
		}
		else {
			adjacentPos = pos.add(1, 0);
		}

		if (!ctx.plane.getTile(adjacentPos).isSupportH) {
			breakTileAndDropItem(ctx, pos);
			return;
		}
	}
	else {
		bool foundAnchor = false;
		Swan::TilePos abovePos = pos;
		while (remainingLength > 0) {
			remainingLength -= 1;
			abovePos = abovePos.add(0, -1);

			auto &aboveTile = ctx.plane.getTile(abovePos);
			auto *rlt = dynamic_cast<RopeLadderTileTrait *>(aboveTile.traits.get());
			if (!rlt) {
				break;
			}

			if (rlt->isAnchor) {
				foundAnchor = true;
				break;
			}
		}

		if (!foundAnchor) {
			ctx.plane.breakTile(pos);
			return;
		}

		// Ensure that the current tile is the right one
		if (remainingLength > 0) {
			ctx.plane.setTile(pos, Swan::cat(
				"core::rope-ladder::middle::", ropeLadderTrait->direction));
		}
	}

	// Spawn tile below
	ctx.plane.nextTick([=](const Swan::Context &ctx) {
		Swan::TilePos belowPos = pos.add(0, 1);
		if (remainingLength > 0 && ctx.plane.getTile(belowPos).name == "@::air") {
			if (remainingLength == 1) {
				ctx.plane.setTile(belowPos, Swan::cat(
					"core::rope-ladder::bottom::", ropeLadderTrait->direction));
			}
			else {
				ctx.plane.setTile(belowPos, Swan::cat(
					"core::rope-ladder::middle::", ropeLadderTrait->direction));
			}
		}
	});
}

void registerRopeLadder(Swan::Mod &mod)
{
	mod.registerTile({
		.name = "rope-ladder",
		.image = "core::tiles/rope-ladder/anchor::left",
		.isSolid = false,
		.droppedItem = "core::rope-ladder",
		.onSpawn = spawnRopeLadderAnchor,
	});

	for (auto direction: {"left", "right"}) {
		mod.registerTile({
			.name = Swan::cat("rope-ladder::anchor::", direction),
			.image = Swan::cat("core::tiles/rope-ladder/anchor::", direction),
			.isSolid = false,
			.droppedItem = "core::rope-ladder",
			.onTileUpdate = cascadeRopeLadder,
			.traits = std::make_shared<RopeLadderTileTrait>(true, direction),
		});
		mod.registerTile({
			.name = Swan::cat("rope-ladder::middle::", direction),
			.image = Swan::cat("core::tiles/rope-ladder/middle::", direction),
			.isSolid = false,
			.isReplacable = true,
			.onTileUpdate = cascadeRopeLadder,
			.traits = std::make_shared<RopeLadderTileTrait>(false, direction),
		});
		mod.registerTile({
			.name = Swan::cat("rope-ladder::bottom::", direction),
			.image = Swan::cat("core::tiles/rope-ladder/bottom::", direction),
			.isSolid = false,
			.isReplacable = true,
			.onTileUpdate = cascadeRopeLadder,
			.traits = std::make_shared<RopeLadderTileTrait>(false, direction),
		});
	}
}

}
