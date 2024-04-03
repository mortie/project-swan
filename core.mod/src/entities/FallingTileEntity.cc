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
	physicsBody_.standardForces();
	physicsBody_.update(ctx, dt);

	if (physicsBody_.onGround) {
		Swan::TilePos pos = {
			(int)floor(physicsBody_.body.midX()),
			(int)floor(physicsBody_.body.midY()),
		};

		ctx.plane.despawnEntity(ctx.plane.currentEntity());
		breakTileAndDropItem(ctx, pos);
		ctx.plane.setTileID(pos, tile_);
	}
}

void FallingTileEntity::serialize(
	const Swan::Context &ctx, MsgStream::MapBuilder &w)
{
	w.writeString("body");
	physicsBody_.serialize(w);
	w.writeString("tile");
	w.writeString(ctx.world.getTileByID(tile_).name);
}

void FallingTileEntity::deserialize(
	const Swan::Context &ctx, MsgStream::MapParser &r)
{
	std::string key;

	while (r.hasNext()) {
		r.nextString(key);

		if (key == "body") {
			physicsBody_.deserialize(r);
		}
		else if (key == "tile") {
			tile_ = ctx.world.getTileID(r.nextString());
		}
		else {
			r.skipNext();
		}
	}
}

}
