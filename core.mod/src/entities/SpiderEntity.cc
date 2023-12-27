#include "SpiderEntity.h"

#include <stdlib.h>

namespace CoreMod {

SpiderEntity::SpiderEntity(const Swan::Context &ctx, Swan::Vec2 pos):
		SpiderEntity(ctx) {
	physicsBody_.body.pos = pos;
}

SpiderEntity::SpiderEntity(const Swan::Context &ctx, const PackObject &obj):
		SpiderEntity(ctx) {
	deserialize(ctx, obj);
}

void SpiderEntity::draw(const Swan::Context &ctx, Cygnet::Renderer &rnd) {
	idleAnimation_.draw(rnd, Cygnet::Mat3gf{}.translate(
		physicsBody_.body.pos - Swan::Vec2{0, 0.35}));
}

void SpiderEntity::update(const Swan::Context &ctx, float dt) {
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

void SpiderEntity::tick(const Swan::Context &ctx, float dt) {
	if (!target_) {
		target_ = ctx.world.player_;
	}

	if (target_ && (target_->center() - physicsBody_.body.center()).squareLength() > 10 * 10) {
		target_ = nullptr;
	}
}

void SpiderEntity::deserialize(const Swan::Context &ctx, const PackObject &obj) {
}

Swan::Entity::PackObject SpiderEntity::serialize(const Swan::Context &ctx, msgpack::zone &zone) {
	return {};
}

}
