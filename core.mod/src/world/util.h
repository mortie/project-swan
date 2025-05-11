#pragma once

#include "entities/ItemStackEntity.h"
#include "entities/FallingTileEntity.h"

#include <swan/swan.h>

namespace CoreMod {

inline void dropItem(
	const Swan::Context &ctx, Swan::TilePos pos, Swan::Item &item)
{
	ctx.plane.entities().spawn<ItemStackEntity>(
		(Swan::Vec2)pos + Swan::Vec2{0.5, 0.5}, &item);
}

inline void dropItem(
	const Swan::Context &ctx, Swan::TilePos pos, const std::string &item)
{
	dropItem(ctx, pos, ctx.world.getItem(item));
}

inline void breakTileAndDropItem(const Swan::Context &ctx, Swan::TilePos pos)
{
	auto &droppedItem = ctx.plane.tiles().get(pos).droppedItem;

	if (droppedItem) {
		dropItem(ctx, pos, *droppedItem);
	}

	ctx.plane.breakTile(pos);
}

inline bool denyIfFloating(const Swan::Context &ctx, Swan::TilePos pos)
{
	auto below = pos + Swan::TilePos{0, 1};

	return ctx.plane.tiles().get(below).isSupportV;
}

inline void breakIfFloating(const Swan::Context &ctx, Swan::TilePos pos)
{
	auto below = pos + Swan::TilePos{0, 1};

	if (!ctx.plane.tiles().get(below).isSupportV) {
		breakTileAndDropItem(ctx, pos);
	}
}

inline void fallIfFloating(const Swan::Context &ctx, Swan::TilePos pos)
{
	auto below = pos + Swan::TilePos{0, 1};

	if (!ctx.plane.tiles().get(below).isSupportV) {
		auto &tile = ctx.plane.tiles().get(pos);
		ctx.plane.tiles().setID(pos, Swan::World::AIR_TILE_ID);
		ctx.plane.entities().spawn<FallingTileEntity>(
			(Swan::Vec2)pos + Swan::Vec2{0.5, 0.5}, tile.id);
	}
}

template<Swan::FixedString NAME, int MAX = 3, int NUM = 1, int DEN = 4>
void dropRandomItemCount(const Swan::Context &ctx, Swan::TilePos pos)
{
	for (int i = 0; i < MAX; ++i) {
		if (Swan::randfloat() > (float(NUM) / float(DEN))) {
			dropItem(ctx, pos, NAME.cStr());
		}
	}
}

}
