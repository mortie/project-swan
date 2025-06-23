#include "SpiderEntity.h"

#include <stdlib.h>

namespace CoreMod {

static constexpr Swan::BasicPhysicsBody::Props PROPS = {
	.size = {1, 0.65},
	.mass = 30,
};
static constexpr float MOVE_FORCE = 50 * PROPS.mass;
static constexpr float JUMP_VEL = 9;

SpiderEntity::SpiderEntity(Swan::Ctx &ctx):
	idleAnimation_(ctx, "core::entities/spider/idle", 0.8),
	physicsBody_(PROPS)
{}

SpiderEntity::SpiderEntity(
	Swan::Ctx &ctx, Swan::Vec2 pos):
	SpiderEntity(ctx)
{
	physicsBody_.body.pos = pos;
}

void SpiderEntity::draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd)
{
	idleAnimation_.draw(rnd, Cygnet::Mat3gf{}.translate(
		physicsBody_.body.pos - Swan::Vec2{0, 0.35}));
}

void SpiderEntity::update(Swan::Ctx &ctx, float dt)
{
	idleAnimation_.tick(dt);

	bool jump = false;

	if (jumpTimer_ > 0) {
		jumpTimer_ -= dt;
		if (jumpTimer_ <= 0) {
			jump = true;
		}
	}

	physicsBody_.standardForces();
	physicsBody_.collideAll(ctx.plane);

	if (target_) {
		auto vec = target_->bottomMid() - physicsBody_.body.center();
		auto direction = vec.norm();
		physicsBody_.force += {direction.x * MOVE_FORCE, 0};

		if (direction.y < 0 && jumpTimer_ <= 0) {
			jumpTimer_ = (rand() % 10) * 0.03 + 0.05;
		}

		if (vec.squareLength() < 1.1 * 1.1) {
			jump = true;
		}
	}

	if (jump && physicsBody_.onGround) {
		physicsBody_.vel.y -= JUMP_VEL;
	}

	physicsBody_.update(ctx, dt);
}

void SpiderEntity::tick(Swan::Ctx &ctx, float dt)
{
	if (!target_) {
		target_ = ctx.world.player_;
	}

	if (target_ && (target_->center() - physicsBody_.body.center()).squareLength() > 10 * 10) {
		target_ = nullptr;
	}
}

void SpiderEntity::serialize(
	Swan::Ctx &ctx, Proto::Builder w)
{
	physicsBody_.serialize(w.initBody());
}

void SpiderEntity::deserialize(
	Swan::Ctx &ctx, Proto::Reader r)
{
	physicsBody_.deserialize(r.getBody());
}

}
