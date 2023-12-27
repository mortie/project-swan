#pragma once

#include "entities/ItemStackEntity.h"

#include <swan/swan.h>

namespace CoreMod {

inline void breakTileAndDropItem(const Swan::Context &ctx, Swan::TilePos pos) {
	auto &droppedItem = ctx.plane.getTile(pos).droppedItem;
	if (droppedItem) {
		ctx.plane.spawnEntity<ItemStackEntity>(
			(Swan::Vec2)pos + Swan::Vec2{0.5, 0.5}, *droppedItem);
	}

	ctx.plane.breakTile(pos);
}

}
