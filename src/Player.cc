#include "Player.h"

namespace Swan {

const float Player::force = 600;
const float Player::friction = 100;
const float Player::mass = 80;
const Vec2 Player::size = Vec2(1, 2);

using Keyboard = sf::Keyboard;

void Player::draw(Win &win) {
	body_.outline(win);
}

void Player::update(float dt) {
	if (Keyboard::isKeyPressed(Keyboard::W) || Keyboard::isKeyPressed(Keyboard::Up))
		body_.force_ += Vec2(0, -force);
	if (Keyboard::isKeyPressed(Keyboard::S) || Keyboard::isKeyPressed(Keyboard::Down))
		body_.force_ += Vec2(0, force);
	if (Keyboard::isKeyPressed(Keyboard::A) || Keyboard::isKeyPressed(Keyboard::Left))
		body_.force_ += Vec2(-force, 0);
	if (Keyboard::isKeyPressed(Keyboard::D) || Keyboard::isKeyPressed(Keyboard::Right))
		body_.force_ += Vec2(force, 0);

	body_.friction(friction);
	body_.gravity();
	body_.update(dt);
}

}
