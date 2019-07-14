#include "Player.h"

void Player::draw(Win &win) {
	body_.outline(win);
}

void Player::update(float dt) {
	body_.vel_.x = 1;
	body_.update(dt);

	if (body_.pos_.x > 20)
		body_.pos_.x = 0;
}
