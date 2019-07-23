#include "EntPlayer.h"

const float EntPlayer::FORCE = 600;
const float EntPlayer::FRICTION = 100;
const float EntPlayer::MASS = 80;
const Swan::Vec2 EntPlayer::SIZE = Swan::Vec2(1, 2);

void EntPlayer::draw(Swan::Win &win) {
	body_.outline(win);
}

void EntPlayer::update(Swan::WorldPlane &plane, float dt) {
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
		body_.force_ += Swan::Vec2(0, -FORCE);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
		body_.force_ += Swan::Vec2(0, FORCE);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
		body_.force_ += Swan::Vec2(-FORCE, 0);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
		body_.force_ += Swan::Vec2(FORCE, 0);

	body_.friction(FRICTION);
	body_.gravity();
	body_.update(dt);
	body_.collide(plane);
}
