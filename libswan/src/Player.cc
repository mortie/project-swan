#include "Player.h"

namespace Swan {

const float Player::FORCE = 600;
const float Player::FRICTION = 100;
const float Player::MASS = 80;
const Vec2 Player::SIZE = Vec2(1, 2);

using Keyboard = sf::Keyboard;

void Player::draw(Win &win) {
	body_.outline(win);
}

void Player::update(WorldPlane &plane, float dt) {
	if (Keyboard::isKeyPressed(Keyboard::W) || Keyboard::isKeyPressed(Keyboard::Up))
		body_.force_ += Vec2(0, -FORCE);
	if (Keyboard::isKeyPressed(Keyboard::S) || Keyboard::isKeyPressed(Keyboard::Down))
		body_.force_ += Vec2(0, FORCE);
	if (Keyboard::isKeyPressed(Keyboard::A) || Keyboard::isKeyPressed(Keyboard::Left))
		body_.force_ += Vec2(-FORCE, 0);
	if (Keyboard::isKeyPressed(Keyboard::D) || Keyboard::isKeyPressed(Keyboard::Right))
		body_.force_ += Vec2(FORCE, 0);

	body_.friction(FRICTION);
	body_.gravity();
	body_.update(dt);
	body_.collide(plane);
}

}
