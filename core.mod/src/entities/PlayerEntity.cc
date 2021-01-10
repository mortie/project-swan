#include "PlayerEntity.h"

#include <cmath>

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
	// body_.outline(win); TODO
	anims_[(int)state_].draw(body_.pos - Swan::Vec2(0.2, 0.1), rnd);
}

void PlayerEntity::update(const Swan::Context &ctx, float dt) {
	State oldState = state_;
	state_ = State::IDLE;

	mouseTile_ = ctx.game.getMouseTile();
	ctx.plane.debugBox(mouseTile_);
	jumpTimer_.tick(dt);
	placeTimer_.tick(dt);

	// Break block
	if (ctx.game.isMousePressed(SDL_BUTTON_LEFT))
		ctx.plane.breakTile(mouseTile_);

	// Place block
	if (ctx.game.isMousePressed(SDL_BUTTON_RIGHT) && placeTimer_.periodic(0.50)) {
		if (ctx.plane.getTileID(mouseTile_) == ctx.world.getTileID("@::air")) {
			ctx.plane.setTile(mouseTile_, "core::torch");
		}
	}

	// Move left
	if (ctx.game.isKeyPressed(SDL_SCANCODE_A) || ctx.game.isKeyPressed(SDL_SCANCODE_LEFT)) {
		physics_.force += Swan::Vec2(-MOVE_FORCE, 0);
		state_ = State::RUNNING_L;
	}

	// Move right
	if (ctx.game.isKeyPressed(SDL_SCANCODE_D) || ctx.game.isKeyPressed(SDL_SCANCODE_RIGHT)) {
		physics_.force += Swan::Vec2(MOVE_FORCE, 0);
		if (state_ == State::RUNNING_L)
			state_ = State::IDLE;
		else
			state_ = State::RUNNING_R;
	}

	bool jumpPressed = ctx.game.isKeyPressed(SDL_SCANCODE_SPACE);

	// Jump
	if (physics_.onGround && jumpPressed && jumpTimer_.periodic(0.5)) {
		physics_.vel.y = -JUMP_VEL;
	}

	// Fall down faster than we went up
	if (!physics_.onGround && (!jumpPressed || physics_.vel.y > 0))
		physics_.force += Swan::Vec2(0, DOWN_FORCE);

	if (state_ != oldState)
		anims_[(int)state_].reset();
	anims_[(int)state_].tick(dt);

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
	for (ItemStackEntity *ent: ctx.plane.getEntsOfType<ItemStackEntity>()) {
		float squared_dist =
			(body_.bottomMid() - ent->get(Swan::BodyTrait::Tag{}).center())
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
