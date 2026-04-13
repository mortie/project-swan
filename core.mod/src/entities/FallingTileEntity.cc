#include "FallingTileEntity.h"

#include "world/util.h"
#include <cstdlib>

namespace CoreMod {

static constexpr Swan::BasicPhysicsBody::Props PROPS = {
	.size = {1, 1},
	.mass = 80,
};

FallingTileEntity::FallingTileEntity(Swan::Ctx &ctx):
	physicsBody_(PROPS)
{}

FallingTileEntity::FallingTileEntity(
	Swan::Ctx &ctx, Swan::Vec2 pos, Swan::Tile::ID tile):
	FallingTileEntity(ctx)
{
	physicsBody_.body.pos = pos;
	tile_ = tile;
}

void FallingTileEntity::draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd)
{
	rnd.drawTile({
		.transform = Cygnet::Mat3gf{}.translate(physicsBody_.body.pos),
		.id = tile_,
	});
}

void FallingTileEntity::update(Swan::Ctx &ctx, float dt)
{
	physicsBody_.collideAll(ctx.plane);

	physicsBody_.standardForces();
	physicsBody_.update(ctx, dt);
	if (physicsBody_.onGround) {
		place(ctx);
	}
}

void FallingTileEntity::place(Swan::Ctx &ctx)
{
	Swan::TilePos bottomPos = {
		(int)floor(physicsBody_.body.midX()),
		(int)floor(physicsBody_.body.bottom()),
	};
	auto &selfTile = ctx.world.getTileByID(tile_);
	auto &bottomTile = ctx.plane.tiles().get(bottomPos);

	bool isSameTileClass = (
		selfTile.id - selfTile.more->baseOffset ==
		bottomTile.id - bottomTile.more->baseOffset);
	if (isSameTileClass) {
		auto *selfStack = dynamic_cast<StackingTileTrait *>(selfTile.more->traits.get());
		auto *bottomStack = dynamic_cast<StackingTileTrait *>(bottomTile.more->traits.get());
		bool bottomHasCapacity = (
			selfStack && bottomStack &&
			bottomStack->stackSize < bottomStack->stackCapacity);
		if (bottomHasCapacity) {
			ctx.plane.tiles().setID(bottomPos, bottomTile.id - 1);
			if (selfStack->stackSize > 1) {
				tile_ += 1;
				float frac = 1.0 / selfStack->stackCapacity;
				physicsBody_.body.size.y -= frac;
			} else {
				ctx.plane.entities().despawn(ctx.plane.entities().current());
			}

			return;
		}
	}

	Swan::TilePos pos = {
		(int)floor(physicsBody_.body.midX()),
		(int)floor(physicsBody_.body.midY()),
	};

	ctx.plane.entities().despawn(ctx.plane.entities().current());
	breakTileAndDropItem(ctx, pos);
	ctx.plane.placeTile(pos, tile_);
	return;
}

void FallingTileEntity::serialize(
	Swan::Ctx &ctx, Proto::Builder w)
{
	physicsBody_.serialize(w.initBody());
	w.setTile(ctx.world.getTileByID(tile_).name);
}

void FallingTileEntity::deserialize(
	Swan::Ctx &ctx, Proto::Reader r)
{
	physicsBody_.deserialize(r.getBody());
	tile_ = ctx.world.getTileID(r.getTile().cStr());
}

}
