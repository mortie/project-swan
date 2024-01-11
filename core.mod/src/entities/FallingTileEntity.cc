#include "FallingTileEntity.h"

#include "world/util.h"

namespace CoreMod {

FallingTileEntity::FallingTileEntity(
	const Swan::Context &ctx, Swan::Vec2 pos, Swan::Tile::ID tile)
{
	physicsBody_.body.pos = pos;
	tile_ = tile;
}

FallingTileEntity::FallingTileEntity(const Swan::Context &ctx, const PackObject &obj)
{
	deserialize(ctx, obj);
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

void FallingTileEntity::deserialize(const Swan::Context &ctx, const PackObject &obj)
{
	physicsBody_.body.pos = obj.at("pos").as<Swan::Vec2>();
	tile_ = obj.at("tile").as<int>();
}

Swan::Entity::PackObject FallingTileEntity::serialize(const Swan::Context &ctx, msgpack::zone &zone)
{
	return {
		{"pos", msgpack::object(physicsBody_.body.pos, zone)},
		{"tile", msgpack::object(tile_, zone)},
	};
}

}
