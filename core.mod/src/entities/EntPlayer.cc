#include "EntPlayer.h"

EntPlayer::EntPlayer(Swan::World &world, const Swan::Vec2 &pos):
		body_(pos, SIZE, MASS) {
	anims_[(int)State::IDLE].init(32, 64, 0.8,
		world.getAsset("core::player-still"));
	anims_[(int)State::RUNNING_R].init(32, 64, 1,
		world.getAsset("core::player-running"));
	anims_[(int)State::RUNNING_L].init(32, 64, 1,
		world.getAsset("core::player-running"), (int)Swan::Animation::Flags::HFLIP);
}

void EntPlayer::draw(Swan::Win &win) {
	body_.outline(win);

	win.setPos(body_.pos_ - Swan::Vec2(0.2, 0.1));
	anims_[(int)state_].draw(win);
}

void EntPlayer::update(Swan::Game &game, Swan::WorldPlane &plane, float dt) {
	State oldState = state_;
	state_ = State::IDLE;

	mouse_tile_ = game.getMouseTile();
	plane.debugBox(mouse_tile_);
	jump_timer_.tick(dt);

	// Break block
	if (game.isMousePressed())
		plane.setTile(mouse_tile_, "core::air");

	// Move left
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
		body_.force_ += Swan::Vec2(-FORCE, 0);
		state_ = State::RUNNING_L;
	}

	// Move right
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
		body_.force_ += Swan::Vec2(FORCE, 0);
		if (state_ == State::RUNNING_L)
			state_ = State::IDLE;
		else
			state_ = State::RUNNING_R;
	}

	// Jump
	if (body_.on_ground_ && sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && jump_timer_.periodic(0.5)) {
		body_.vel_.y_ = -JUMP_FORCE;
	}

	if (state_ != oldState)
		anims_[(int)state_].reset();
	anims_[(int)state_].tick(dt);

	body_.friction(FRICTION);
	body_.gravity();
	body_.update(dt);
	body_.collide(plane);
}
