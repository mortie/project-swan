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

		ctx.plane.despawnEntity(ctx.plane.currentEntity());
		breakTileAndDropItem(ctx, pos);
		ctx.plane.placeTile(pos, tile_);
		return;
	}
}

void FallingTileEntity::serialize(
	const Swan::Context &ctx, sbon::ObjectWriter w)
{
	physicsBody_.serialize(w.key("body"));
	w.key("tile").writeString(ctx.world.getTileByID(tile_).name);
}

void FallingTileEntity::deserialize(
	const Swan::Context &ctx, sbon::ObjectReader r)
{
	r.match({
		{"body", [&](sbon::Reader val) {
			physicsBody_.deserialize(val);
		}},
		{"tile", [&](sbon::Reader val) {
			tile_ = ctx.world.getTileID(val.getString());
		}},
	});
}

}
