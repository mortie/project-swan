#include "PlayerEntity.h"

#include <cmath>

#include "ItemStackEntity.h"

PlayerEntity::PlayerEntity(const Swan::Context &ctx, Swan::Vec2 pos):
		PlayerEntity(ctx) {
	body_.pos_ = pos;
}

PlayerEntity::PlayerEntity(const Swan::Context &ctx, const PackObject &obj):
		PlayerEntity(ctx) {
	deserialize(ctx, obj);
}

void PlayerEntity::draw(const Swan::Context &ctx, Swan::Win &win) {
	body_.outline(win);
	anims_[(int)state_].draw(body_.pos_ - Swan::Vec2(0.2, 0.1), win);
}

void PlayerEntity::update(const Swan::Context &ctx, float dt) {
	State oldState = state_;
	state_ = State::IDLE;

	mouse_tile_ = ctx.game.getMouseTile();
	ctx.plane.debugBox(mouse_tile_);
	jump_timer_.tick(dt);

	// Break block
	if (ctx.game.isMousePressed(SDL_BUTTON_LEFT))
		ctx.plane.breakTile(mouse_tile_);

	// Move left
	if (ctx.game.isKeyPressed(SDL_SCANCODE_A) || ctx.game.isKeyPressed(SDL_SCANCODE_LEFT)) {
		body_.force_ += Swan::Vec2(-MOVE_FORCE, 0);
		state_ = State::RUNNING_L;
	}

	// Move right
	if (ctx.game.isKeyPressed(SDL_SCANCODE_D) || ctx.game.isKeyPressed(SDL_SCANCODE_RIGHT)) {
		body_.force_ += Swan::Vec2(MOVE_FORCE, 0);
		if (state_ == State::RUNNING_L)
			state_ = State::IDLE;
		else
			state_ = State::RUNNING_R;
	}

	bool jump_pressed = ctx.game.isKeyPressed(SDL_SCANCODE_SPACE);

	// Jump
	if (body_.on_ground_ && jump_pressed && jump_timer_.periodic(0.5)) {
		body_.vel_.y = -JUMP_VEL;
	}

	// Fall down faster than we went up
	if (!body_.on_ground_ && (!jump_pressed || body_.vel_.y > 0))
		body_.force_ += Swan::Vec2(0, DOWN_FORCE);

	if (state_ != oldState)
		anims_[(int)state_].reset();
	anims_[(int)state_].tick(dt);

	PhysicsEntity::update(ctx, dt);
}

void PlayerEntity::tick(const Swan::Context &ctx, float dt) {
	for (ItemStackEntity *ent: ctx.plane.getEntsOfType<ItemStackEntity>()) {
		float squared_dist =
			(getBody().getBounds().bottomMid() - ent->getBody().getBounds().center())
			.squareLength();

		if (squared_dist < 0.5 * 0.5) {
			// TODO: Pick up
		}
	}
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
