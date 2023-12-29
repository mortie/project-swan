#include "ladder.h"

#include "util.h"

namespace CoreMod {

void cascadeRopeLadder(const Swan::Context &ctx, Swan::TilePos pos)
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
		Swan::TilePos leftPos = pos.add(-1, 0);
		if (ctx.plane.getTile(leftPos).name == "@::air") {
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
	Swan::TilePos belowPos = pos.add(0, 1);
	if (remainingLength > 0 && ctx.plane.getTile(belowPos).name == "@::air") {
		if (remainingLength == 1) {
			ctx.plane.setTile(belowPos, "core::rope-ladder-bottom");
		}
		else {
			ctx.plane.setTile(belowPos, "core::rope-ladder-middle");
		}
	}
}

}
