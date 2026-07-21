#include "DynamiteEntity.h"

#include <cmath>

#include "swan/common.h"
#include "world/util.h"

namespace CoreMod {

static constexpr Swan::BasicPhysicsBody::Props PROPS = {
	.size = {0.6, 0.2},
	.mass = 30,
	.isSolid = false,
};
static constexpr float FUSE_TIME = 4;
static constexpr int RADIUS = 10;

static void explode(Swan::Ctx &ctx, Swan::Vec2 pos)
{
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

	Swan::TilePos base = {(int)round(pos.x), (int)round(pos.y)};
	auto consider = [&](Swan::Vec2i offset) {
		Swan::TilePos pos = base + offset;
		float dist = offset.as<float>().scale(0.75, 1.0).length();
		if (dist > RADIUS) {
			return;
		}

		auto &tile = ctx.plane.tiles().get(pos);
		float explosionForce = 1.0 / (dist + 1);
		if (explosionForce < tile.more->explosionResistance) {
			return;
		}

		auto dir = base - pos;
		auto cast = ctx.plane.tiles().raycast(pos.as<float>().add(0.5, 0.5), dir, dist);
		if (cast.hit) {
			return;
		}

		if (tile.breakableBy.contains(Swan::Tool::HAND)) {
			breakTileAndDropItem(ctx, pos);
		} else {
			ctx.plane.tiles().setID(pos, Swan::World::AIR_TILE_ID);
		}
	};

	consider({0, 0});
	for (int r = 1; r <= RADIUS; ++r) {
		for (int x = -r; x <= r; ++x) {
			consider({x, -r});
			consider({x, r});
		}

		for (int y = -r; y <= r; ++y) {
			consider({-r, y});
			consider({r, y});
		}
	}

	Swan::BodyTrait::Body body = {
		.pos = pos.add(-RADIUS, -RADIUS),
		.size = {RADIUS * 2 + 1, RADIUS * 2 + 1},
	};

	for (auto &collision: ctx.plane.entities().getColliding(body)) {
		auto delta = collision.body.center() - pos;

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
		.translate({-0.05, -0.275}));
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
		.pos = physicsBody_.body.midRight().add(0.01, -0.16),
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
