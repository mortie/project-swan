#include "PlayerEntity.h"

#include <cmath>
#include <compare>

#include "ItemStackEntity.h"

PlayerEntity::PlayerEntity(const Swan::Context &ctx, Swan::Vec2 pos):
		PlayerEntity(ctx) {
	body_.pos = pos;
}

PlayerEntity::PlayerEntity(const Swan::Context &ctx, const PackObject &obj):
		PlayerEntity(ctx) {
	deserialize(ctx, obj);
}

void PlayerEntity::draw(const Swan::Context &ctx, Cygnet::Renderer &rnd) {
	Cygnet::Mat3gf mat;

	// Currently, there is no sprite for running left.
	// Running left is just running right but flipped.
	if (state_ == State::RUNNING_L) {
		mat.translate({-0.5, 0}).scale({-1, 1}).translate({0.5, 0});
	}

	currentAnimation_->draw(rnd, mat.translate(
		body_.pos - Swan::Vec2{0.2, 0.1}));

	rnd.drawRect({mouseTile_, {1, 1}});
}

void PlayerEntity::update(const Swan::Context &ctx, float dt) {
	State oldState = state_;
	state_ = State::IDLE;

	mouseTile_ = ctx.game.getMouseTile();
	jumpTimer_.tick(dt);
	placeTimer_.tick(dt);

	// Break block
	if (ctx.game.isMousePressed(GLFW_MOUSE_BUTTON_LEFT))
		ctx.plane.breakTile(mouseTile_);

	// Place block
	if (ctx.game.isMousePressed(GLFW_MOUSE_BUTTON_RIGHT) && placeTimer_.periodic(0.50)) {
		if (ctx.plane.getTileID(mouseTile_) == ctx.world.getTileID("@::air")) {
			ctx.plane.setTile(mouseTile_, "core::torch");
		}
	}

	// Move left
	if (ctx.game.isKeyPressed(GLFW_KEY_A) || ctx.game.isKeyPressed(GLFW_KEY_LEFT)) {
		physics_.force += Swan::Vec2(-MOVE_FORCE, 0);
		state_ = State::RUNNING_L;
	}

	// Move right
	if (ctx.game.isKeyPressed(GLFW_KEY_D) || ctx.game.isKeyPressed(GLFW_KEY_RIGHT)) {
		physics_.force += Swan::Vec2(MOVE_FORCE, 0);
		if (state_ == State::RUNNING_L)
			state_ = State::IDLE;
		else
			state_ = State::RUNNING_R;
	}

	bool jumpPressed = ctx.game.isKeyPressed(GLFW_KEY_SPACE);

	// Jump
	if (physics_.onGround && jumpPressed && jumpTimer_.periodic(0.5)) {
		physics_.vel.y = -JUMP_VEL;
	}

	// Fall down faster than we went up
	if (!physics_.onGround && (!jumpPressed || physics_.vel.y > 0))
		physics_.force += Swan::Vec2(0, DOWN_FORCE);

	if (state_ != oldState) {
		switch (state_) {
		case State::IDLE:
			currentAnimation_ = &idleAnimation_;
			break;
		case State::RUNNING_L:
		case State::RUNNING_R:
			currentAnimation_ = &runningAnimation_;
			break;
		}
		currentAnimation_->reset();
	}
	currentAnimation_->tick(dt);

	if (invincibleTimer_ >= 0) {
		invincibleTimer_ -= dt;
	}

	// Collide with stuff
	auto &collisions = ctx.plane.getCollidingEntities(ctx.plane.currentEntity(), body_);
	for (auto &c: collisions) {
		auto *entity = c.ref.get();

		// Get damaged if it's something which deals contact damage
		auto *damage = entity->trait<Swan::ContactDamageTrait>();
		bool damaged = false;
		if (damage && invincibleTimer_ <= 0) {
			Swan::Vec2 direction;
			direction.y = -0.5;
			if (body_.center().x < c.body->center().x) {
				direction.x = -1;
			} else {
				direction.x = 1;
			}

			physics_.vel += direction * damage->knockback;
			damaged = true;
		}

		if (damaged) {
			invincibleTimer_ = 0.5;
		}

		// Pick it up if it's an item stack
		auto *itemStackEnt = dynamic_cast<ItemStackEntity *>(entity);
		if (itemStackEnt) {
			Swan::ItemStack stack{itemStackEnt->item(), 1};
			stack = inventory_.insert(0, stack);
			if (stack.empty()) {
				ctx.plane.despawnEntity(c.ref);
			}
		}
	}

	physics(ctx, dt, { .mass = MASS });

	// Do this after moving so that it's not behind
	Swan::Vec2 headPos = body_.topMid() + Swan::Vec2(0, 0.5);
	Swan::TilePos tilePos = Swan::Vec2i(floor(headPos.x), floor(headPos.y));
	if (!placedLight_) {
		ctx.plane.addLight(tilePos, LIGHT_LEVEL);
		placedLight_ = true;
		lightTile_ = tilePos;
	} else if (tilePos != lightTile_) {
		ctx.plane.removeLight(lightTile_, LIGHT_LEVEL);
		ctx.plane.addLight(tilePos, LIGHT_LEVEL);
		lightTile_ = tilePos;
	}
}

void PlayerEntity::tick(const Swan::Context &ctx, float dt) {
}

void PlayerEntity::deserialize(const Swan::Context &ctx, const PackObject &obj) {
	//body_.deserialize(obj["body"]);
}

Swan::Entity::PackObject PlayerEntity::serialize(const Swan::Context &ctx, msgpack::zone &zone) {
	return {};
	/*
	return Swan::MsgPackObject{
		{ "body", body_.serialize(w) },
	};
	*/
}
