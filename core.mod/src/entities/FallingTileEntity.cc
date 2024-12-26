#include "FallingTileEntity.h"

#include "world/util.h"

namespace CoreMod {

FallingTileEntity::FallingTileEntity(
	const Swan::Context &ctx, Swan::Vec2 pos, Swan::Tile::ID tile)
{
	physicsBody_.body.pos = pos;
	tile_ = tile;
}

void FallingTileEntity::draw(const Swan::Context &ctx, Cygnet::Renderer &rnd)
{
	rnd.drawTile({
		.transform = Cygnet::Mat3gf{}.translate(physicsBody_.body.pos),
		.id = tile_,
	});
}

void FallingTileEntity::update(const Swan::Context &ctx, float dt)
{
	physicsBody_.collideAll(ctx.plane);

	physicsBody_.standardForces();
	physicsBody_.update(ctx, dt);

	// Normally, we would check onGround after collideAll
	// and before update, to treat other entities as ground.
	// However, here we only want to treat terrain as ground,
	// so we let physicsBody_.update clear the onGround state
	// from collideAll first.
	if (physicsBody_.onGround) {
		Swan::TilePos pos = {
			(int)floor(physicsBody_.body.midX()),
			(int)floor(physicsBody_.body.midY()),
		};

		ctx.plane.entities().despawn(ctx.plane.entities().current());
		breakTileAndDropItem(ctx, pos);
		ctx.plane.placeTile(pos, tile_);
		return;
	}
}

void FallingTileEntity::serialize(
	const Swan::Context &ctx, Proto::Builder w)
{
	physicsBody_.serialize(w.initBody());
	w.setTile(ctx.world.getTileByID(tile_).name);
}

void FallingTileEntity::deserialize(
	const Swan::Context &ctx, Proto::Reader r)
{
	physicsBody_.deserialize(r.getBody());
	tile_ = ctx.world.getTileID(r.getTile().cStr());
}

}
