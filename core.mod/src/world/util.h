#pragma once

#include "entities/ItemStackEntity.h"

#include <swan/swan.h>

namespace CoreMod {

inline void dropItem(const Swan::Context &ctx, Swan::TilePos pos, const std::string &item)
{
	ctx.plane.spawnEntity<ItemStackEntity>(
		(Swan::Vec2)pos + Swan::Vec2{0.5, 0.5}, item);
}

inline void breakTileAndDropItem(const Swan::Context &ctx, Swan::TilePos pos)
{
	auto &droppedItem = ctx.plane.getTile(pos).droppedItem;

	if (droppedItem) {
		dropItem(ctx, pos, *droppedItem);
	}

	ctx.plane.breakTile(pos);
}

inline void breakIfFloating(const Swan::Context &ctx, Swan::TilePos pos)
{
	auto below = pos + Swan::TilePos{0, 1};

	if (ctx.plane.getTile(below).name == "@::air") {
		breakTileAndDropItem(ctx, pos);
	}
}

}
