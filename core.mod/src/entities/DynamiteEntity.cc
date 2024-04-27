#include "DynamiteEntity.h"

#include <cmath>

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

	float vy = std::abs(x) * -6;

	ctx.plane.setTileID(tp, Swan::World::AIR_TILE_ID);
	auto ref = ctx.plane.spawnEntity<FallingTileEntity>(
		(Swan::Vec2)tp + Swan::Vec2{0.5, 0.5}, id);
	auto *body = ref.trait<Swan::PhysicsBodyTrait>();
	body->addVelocity({vx, vy});
}

static void explode(const Swan::Context &ctx, Swan::Vec2 pos)
{
	constexpr int R1 = 2;
	constexpr int R2 = 3;
	constexpr int R3 = 6;

	for (int y = -R2; y <= R2; ++y) {
		for (int x = -R2; x <= R2; ++x) {
			Swan::TilePos tp = {(int)round(pos.x + x), (int)round(pos.y + y)};
			float squareDist = y * y + x * x;

			if (squareDist <= R1 * R1) {
				breakTileAndDropItem(ctx, tp);
			}
			else if (squareDist <= R2 * R2) {
				explodeTile(ctx, tp, x, y);
			}
		}
	}

	Swan::BodyTrait::Body body = {
		.pos = pos.add(-R3, -R3),
		.size = {R3 * 2, R3 * 2},
	};

	for (auto &collision: ctx.plane.getCollidingEntities(body)) {
		auto delta = collision.body.center() - pos;
		if (delta.squareLength() > R3 * R3) {
			continue;
		}

		Swan::Vec2 vel = delta.norm() * (20.0 / std::max(delta.length(), 1.0f));
		collision.ref.traitThen<Swan::PhysicsBodyTrait>([&](auto &body) {
			body.addVelocity(vel);
		});
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
	const Swan::Context &ctx, sbon::ObjectWriter w)
{
	physicsBody_.serialize(w.key("body"));
	w.key("fuse").writeFloat(fuse_);
}

void DynamiteEntity::deserialize(
	const Swan::Context &ctx, sbon::ObjectReader r)
{
	r.all([&](std::string &key, sbon::Reader val) {
		if (key == "body") {
			physicsBody_.deserialize(val);
		}
		else if (key == "fuse") {
			fuse_ = val.getFloat();
		}
		else {
			val.skip();
		}
	});
}

}
