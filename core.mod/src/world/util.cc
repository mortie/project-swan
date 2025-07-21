#include "util.h"

#include "entities/ItemStackEntity.h"
#include "entities/PlayerEntity.h"
#include "entities/FallingTileEntity.h"

namespace CoreMod {

void dropItem(
	Swan::Ctx &ctx, Swan::TilePos pos, Swan::Item &item)
{
	ctx.plane.entities().spawn<ItemStackEntity>(
		(Swan::Vec2)pos + Swan::Vec2{0.5, 0.5}, &item);
}

void dropItem(
	Swan::Ctx &ctx, Swan::TilePos pos, const std::string &item)
{
	dropItem(ctx, pos, ctx.world.getItem(item));
}

void breakTileAndDropItem(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto &droppedItem = ctx.plane.tiles().get(pos).more->droppedItem;

	if (droppedItem) {
		dropItem(ctx, pos, *droppedItem);
	}

	ctx.plane.breakTile(pos);
}

bool denyIfFloating(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto below = pos + Swan::TilePos{0, 1};

	return ctx.plane.tiles().get(below).isSupportV();
}

void breakIfFloating(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto below = pos + Swan::TilePos{0, 1};

	if (!ctx.plane.tiles().get(below).isSupportV()) {
		breakTileAndDropItem(ctx, pos);
	}
}

void fallIfFloating(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto below = pos + Swan::TilePos{0, 1};

	if (!ctx.plane.tiles().get(below).isSupportV()) {
		auto &tile = ctx.plane.tiles().get(pos);
		ctx.plane.tiles().setID(pos, Swan::World::AIR_TILE_ID);
		ctx.plane.entities().spawn<FallingTileEntity>(
			(Swan::Vec2)pos + Swan::Vec2{0.5, 0.5}, tile.id);
	}
}

bool healPlayer(Swan::Ctx &ctx, Swan::EntityRef playerRef, int n)
{
	auto player = dynamic_cast<PlayerEntity *>(playerRef.get());
	if (!player) {
		return false;
	}

	return player->heal(ctx, n);
}

}
