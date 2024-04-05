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

static void spawnRopeLadderAnchor(const Swan::Context &ctx, Swan::TilePos pos)
{
	if (ctx.plane.getTile(pos.add(-1, 0)).isSupport) {
		ctx.plane.setTile(pos, "core::rope-ladder::anchor::left");
	}
	else if (ctx.plane.getTile(pos.add(1, 0)).isSupport) {
		ctx.plane.setTile(pos, "core::rope-ladder::anchor::right");
	}
	else {
		breakTileAndDropItem(ctx, pos);
	}
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

		if (!ctx.plane.getTile(adjacentPos).isSupport) {
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
			.onTileUpdate = cascadeRopeLadder,
			.traits = std::make_shared<RopeLadderTileTrait>(false, direction),
		});
		mod.registerTile({
			.name = Swan::cat("rope-ladder::bottom::", direction),
			.image = Swan::cat("core::tiles/rope-ladder/bottom::", direction),
			.isSolid = false,
			.onTileUpdate = cascadeRopeLadder,
			.traits = std::make_shared<RopeLadderTileTrait>(false, direction),
		});
	}
}

}
