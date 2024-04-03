#pragma once

#include "entities/ItemStackEntity.h"
#include "entities/FallingTileEntity.h"

#include <swan/swan.h>

namespace CoreMod {

inline void dropItem(
	const Swan::Context &ctx, Swan::TilePos pos, Swan::Item &item)
{
	ctx.plane.spawnEntity<ItemStackEntity>(
		(Swan::Vec2)pos + Swan::Vec2{0.5, 0.5}, &item);
}

inline void dropItem(
	const Swan::Context &ctx, Swan::TilePos pos, const std::string &item)
{
	dropItem(ctx, pos, ctx.world.getItem(item));
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

	if (!ctx.plane.getTile(below).isOpaque) {
		breakTileAndDropItem(ctx, pos);
	}
}

inline void fallIfFloating(const Swan::Context &ctx, Swan::TilePos pos)
{
	auto below = pos + Swan::TilePos{0, 1};

	if (!ctx.plane.getTile(below).isSolid) {
		auto &tile = ctx.plane.getTile(pos);
		ctx.plane.breakTile(pos);
		ctx.plane.spawnEntity<FallingTileEntity>(
			(Swan::Vec2)pos + Swan::Vec2{0.5, 0.5}, tile.id);
	}
}

}
