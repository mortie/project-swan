#include "DynamiteEntity.h"

#include <math.h>

#include "FallingTileEntity.h"
#include "world/util.h"

namespace CoreMod {

static void explode(const Swan::Context &ctx, Swan::Vec2 pos)
{
	constexpr int R1 = 4;
	constexpr int R2 = 6;

	for (int y = -20; y <= 20; ++y) {
		for (int x = -20; x <= 20; ++x) {
			Swan::TilePos tp = {(int)round(pos.x + x), (int)round(pos.y + y)};
			float dist = sqrt(y * y + x * x);
			if (dist <= R1) {
				breakTileAndDropItem(ctx, tp);
			}
			else if (dist <= R2) {
				auto id = ctx.plane.getTileID(tp);
				ctx.plane.breakTile(tp);
				ctx.plane.spawnEntity<FallingTileEntity>(
					(Swan::Vec2)tp + Swan::Vec2{0.5, 0.5}, id);
			}
		}
	}
}

DynamiteEntity::DynamiteEntity(
	const Swan::Context &ctx, Swan::Vec2 pos, Swan::Vec2 vel)
{
	physicsBody_.body.pos = pos;
	physicsBody_.vel = vel;
	tile_ = ctx.world.getItem("core::dynamite").id;
}

DynamiteEntity::DynamiteEntity(
	const Swan::Context &ctx, MsgStream::MapParser &r)
{
	deserialize(ctx, r);
	tile_ = ctx.world.getTileID("core::dynamite");
}

void DynamiteEntity::draw(const Swan::Context &ctx, Cygnet::Renderer &rnd)
{
	rnd.drawTile({
		.transform = Cygnet::Mat3gf{}
			.scale({0.75, 0.75})
			.translate(physicsBody_.body.pos)
			.translate({0, -0.25}),
		.id = tile_,
	});
}

void DynamiteEntity::update(const Swan::Context &ctx, float dt)
{
	physicsBody_.standardForces();
	physicsBody_.update(ctx, dt);

	fuse_ -= dt;
	if (fuse_ <= 0) {
		ctx.plane.despawnEntity(ctx.plane.currentEntity());
		explode(ctx, physicsBody_.body.center());
	}
}

void DynamiteEntity::deserialize(
	const Swan::Context &ctx, MsgStream::MapParser &r)
{}

void DynamiteEntity::serialize(
	const Swan::Context &ctx, MsgStream::MapBuilder &w)
{}

}
