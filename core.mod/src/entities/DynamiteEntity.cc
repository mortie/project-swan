#include "DynamiteEntity.h"

#include <cmath>

#include "swan/common.h"
#include "world/util.h"

namespace CoreMod {

static constexpr Swan::BasicPhysicsBody::Props PROPS = {
	.size = {0.8, 0.2},
	.mass = 30,
	.isSolid = false,
};
static constexpr float FUSE_TIME = 4;

static void explode(Swan::Ctx &ctx, Swan::Vec2 pos)
{
	constexpr float R1 = 2;
	constexpr float R2 = 3;
	constexpr float R3 = 6;

	for (int i = 0; i < 60; ++i) {
		ctx.game.spawnParticle({
			.pos = pos + Swan::Vec2{
				(Swan::randfloat() - 0.5f) * 3.0f,
				(Swan::randfloat() - 0.5f) * 2.0f,
			},
			.vel = {
				(Swan::randfloat() * 4 - 2),
				Swan::randfloat() * -4,
			},
			.color = {0.3f, 0.3f, 0.3f, Swan::randfloat() * 0.2f + 0.75f},
			.lifetime = 0.4f + Swan::randfloat() * 0.2f,
			.weight = 0.5,
		});
	}

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

DynamiteEntity::DynamiteEntity(Swan::Ctx &ctx):
	animation_(ctx, "core::misc/burning-dynamite", 0.1),
	fuse_(FUSE_TIME),
	physicsBody_(PROPS)
{}

DynamiteEntity::DynamiteEntity(
	Swan::Ctx &ctx, Swan::Vec2 pos, Swan::Vec2 vel):
	DynamiteEntity(ctx)
{
	physicsBody_.body.pos = pos;
	physicsBody_.vel = vel;
	fuseSound_ = Swan::SoundHandle::make();
	ctx.game.playSound(
		ctx.world.getSound("core::misc/fuse"),
		physicsBody_.body.center(), fuseSound_);

}

void DynamiteEntity::draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd)
{
	animation_.draw(rnd, Cygnet::Mat3gf{}
		.scale({0.75, 0.75})
		.translate(physicsBody_.body.pos)
		.translate({0, -0.25}));
}

void DynamiteEntity::update(Swan::Ctx &ctx, float dt)
{
	physicsBody_.standardForces();
	physicsBody_.update(ctx, dt);
	animation_.tick(dt);

	fuseSound_.move(physicsBody_.body.center());

	fuse_ -= dt;
	if (fuse_ <= 0) {
		ctx.game.playSound(
			ctx.world.getSound("core::misc/explosion"),
			physicsBody_.body.center());
		fuseSound_.stop();
		ctx.plane.entities().despawn(ctx.plane.entities().current());
		explode(ctx, physicsBody_.body.center());
	}
}

void DynamiteEntity::tick(Swan::Ctx &ctx, float dt)
{
	ctx.game.spawnParticle({
		.pos = physicsBody_.body.midRight().add(-0.13, -0.15),
		.vel = {(Swan::randfloat() - 0.5f) * 1, -(Swan::randfloat() * 0.5f + 0.6f)},
		.size = {1.0f / 16, 1.0f / 16},
		.color = {0.3f, 0.3f, 0.3f, Swan::randfloat() * 0.2f + 0.75f},
		.lifetime = Swan::randfloat() + 0.2f,
		.weight = 0.05,
	});
}

void DynamiteEntity::serialize(
	Swan::Ctx &ctx, Proto::Builder w)
{
	physicsBody_.serialize(w.initBody());
	w.setFuse(fuse_);
}

void DynamiteEntity::deserialize(
	Swan::Ctx &ctx, Proto::Reader r)
{
	physicsBody_.deserialize(r.getBody());
	fuse_ = r.getFuse();
}

}
