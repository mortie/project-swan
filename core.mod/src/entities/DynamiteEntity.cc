#include "DynamiteEntity.h"

#include <cmath>

#include "swan/common.h"
#include "world/util.h"

namespace CoreMod {

static constexpr Swan::BasicPhysicsBody::Props PROPS = {
	.size = {0.8, 0.2},
	.mass = 20,
	.isSolid = false,
};
static constexpr float FUSE_TIME = 5;

static void explode(const Swan::Context &ctx, Swan::Vec2 pos)
{
	constexpr float R1 = 2;
	constexpr float R2 = 3;
	constexpr float R3 = 6;

	for (int y = -R2; y <= R2; ++y) {
		for (int x = -R2; x <= R2; ++x) {
			Swan::TilePos tp = {(int)round(pos.x + x - 0.5), (int)round(pos.y + y - 0.5)};
			float squareDist = y * y + (x * x * 0.5);

			if (squareDist <= R1 * R1) {
				ctx.plane.tiles().setID(tp, Swan::World::AIR_TILE_ID);
			}
			else if (squareDist <= R2 * R2) {
				auto &tile = ctx.plane.tiles().get(tp);
				if (tile.breakableBy.contains(Swan::Tool::HAND)) {
					breakTileAndDropItem(ctx, tp);
				}
			}
		}
	}

	Swan::BodyTrait::Body body = {
		.pos = pos.add(-R3, -R3),
		.size = {R3 * 2, R3 * 2},
	};

	for (auto &collision: ctx.plane.entities().getColliding(body)) {
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

DynamiteEntity::DynamiteEntity(const Swan::Context &ctx):
	tile_(ctx.world.getItem("core::dynamite").id),
	fuse_(FUSE_TIME),
	physicsBody_(PROPS)
{}

DynamiteEntity::DynamiteEntity(
	const Swan::Context &ctx, Swan::Vec2 pos, Swan::Vec2 vel):
	DynamiteEntity(ctx)
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
		ctx.game.playSound(
			ctx.world.getSound("core::sounds/misc/explosion"),
			physicsBody_.body.center());
		ctx.plane.entities().despawn(ctx.plane.entities().current());
		explode(ctx, physicsBody_.body.center());
	}
}

void DynamiteEntity::serialize(
	const Swan::Context &ctx, Proto::Builder w)
{
	physicsBody_.serialize(w.initBody());
	w.setFuse(fuse_);
}

void DynamiteEntity::deserialize(
	const Swan::Context &ctx, Proto::Reader r)
{
	physicsBody_.deserialize(r.getBody());
	fuse_ = r.getFuse();
}

}
