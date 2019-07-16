#include "Player.h"

using Keyboard = sf::Keyboard;

void Player::draw(Win &win) {
	body_.outline(win);
}

void Player::update(float dt) {
	if (Keyboard::isKeyPressed(Keyboard::W) || Keyboard::isKeyPressed(Keyboard::Up))
		body_.force_.y -= force;
	if (Keyboard::isKeyPressed(Keyboard::S) || Keyboard::isKeyPressed(Keyboard::Down))
		body_.force_.y += force;
	if (Keyboard::isKeyPressed(Keyboard::A) || Keyboard::isKeyPressed(Keyboard::Left))
		body_.force_.x -= force;
	if (Keyboard::isKeyPressed(Keyboard::D) || Keyboard::isKeyPressed(Keyboard::Right))
		body_.force_.x += force;

	body_.friction(friction);
	body_.gravity();
	body_.update(dt);
}
