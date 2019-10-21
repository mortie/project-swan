#include "EntPlayer.h"

EntPlayer::EntPlayer(const Swan::Context &ctx, const Swan::SRF &params):
		body_(SIZE, MASS) {

	readSRF(ctx, params);

	anims_[(int)State::IDLE].init(32, 64, 0.8,
		ctx.world.getAsset("core::player-still"));
	anims_[(int)State::RUNNING_R].init(32, 64, 1,
		ctx.world.getAsset("core::player-running"));
	anims_[(int)State::RUNNING_L].init(32, 64, 1,
		ctx.world.getAsset("core::player-running"), (int)Swan::Animation::Flags::HFLIP);
}

void EntPlayer::draw(const Swan::Context &ctx, Swan::Win &win) {
	body_.outline(win);

	win.setPos(body_.pos_ - Swan::Vec2(0.2, 0.1));
	anims_[(int)state_].draw(win);
}

void EntPlayer::update(const Swan::Context &ctx, float dt) {
	State oldState = state_;
	state_ = State::IDLE;

	mouse_tile_ = ctx.game.getMouseTile();
	ctx.plane.debugBox(mouse_tile_);
	jump_timer_.tick(dt);

	// Break block
	if (ctx.game.isMousePressed(sf::Mouse::Button::Left))
		ctx.plane.breakBlock(mouse_tile_);

	// Move left
	if (ctx.game.isKeyPressed(sf::Keyboard::A) || ctx.game.isKeyPressed(sf::Keyboard::Left)) {
		body_.force_ += Swan::Vec2(-FORCE, 0);
		state_ = State::RUNNING_L;
	}

	// Move right
	if (ctx.game.isKeyPressed(sf::Keyboard::D) || ctx.game.isKeyPressed(sf::Keyboard::Right)) {
		body_.force_ += Swan::Vec2(FORCE, 0);
		if (state_ == State::RUNNING_L)
			state_ = State::IDLE;
		else
			state_ = State::RUNNING_R;
	}

	// Jump
	if (body_.on_ground_ && ctx.game.isKeyPressed(sf::Keyboard::Space) && jump_timer_.periodic(0.5)) {
		body_.vel_.y = -JUMP_FORCE;
	}

	if (state_ != oldState)
		anims_[(int)state_].reset();
	anims_[(int)state_].tick(dt);

	body_.friction(FRICTION);
	body_.gravity();
	body_.update(dt);
	body_.collide(ctx.plane);
}

void EntPlayer::readSRF(const Swan::Context &ctx, const Swan::SRF &srf) {
	auto pos = dynamic_cast<const Swan::SRFFloatArray &>(srf);
	body_.pos_.set(pos.val[0], pos.val[1]);
}

Swan::SRF *EntPlayer::writeSRF(const Swan::Context &ctx) {
	return new Swan::SRFFloatArray{ body_.pos_.x, body_.pos_.y };
}
