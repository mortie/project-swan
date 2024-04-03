#include "DynamiteEntity.h"

#include <math.h>

#include "FallingTileEntity.h"
#include "swan/common.h"
#include "world/util.h"

namespace CoreMod {

static void explodeTile(const Swan::Context &ctx, Swan::TilePos tp, int x, int y)
{
	auto id = ctx.plane.getTileID(tp);

	if (id == Swan::World::AIR_TILE_ID) {
		return;
	}

	float vx = 0;
	if (x != 0) {
		vx = 1.0 / x;
	}

	float vy = abs(x) * -6;

	ctx.plane.setTileID(tp, Swan::World::AIR_TILE_ID);
	auto ref = ctx.plane.spawnEntity<FallingTileEntity>(
		(Swan::Vec2)tp + Swan::Vec2{0.5, 0.5}, id);
	auto *body = ref.trait<Swan::PhysicsBodyTrait>();
	body->addVelocity({vx, vy});
}

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
				explodeTile(ctx, tp, x, y);
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

void DynamiteEntity::serialize(
	const Swan::Context &ctx, MsgStream::MapBuilder &w)
{
	w.writeString("body");
	physicsBody_.serialize(w);
	w.writeString("fuse");
	w.writeFloat32(fuse_);
}

void DynamiteEntity::deserialize(
	const Swan::Context &ctx, MsgStream::MapParser &r)
{
	std::string key;

	while (r.hasNext()) {
		r.nextString(key);

		if (key == "body") {
			physicsBody_.deserialize(r);
		}
		else if (key == "fuse") {
			fuse_ = r.nextFloat32();
		}
		else {
			r.skipNext();
		}
	}
}

}
