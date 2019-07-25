#include "EntPlayer.h"

void EntPlayer::draw(Swan::Win &win) {
	body_.outline(win);
}

void EntPlayer::update(Swan::WorldPlane &plane, float dt) {
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
		body_.force_ += Swan::Vec2(-FORCE, 0);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
		body_.force_ += Swan::Vec2(FORCE, 0);
	if (body_.on_ground_ && sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		body_.vel_.y_ = -JUMP_FORCE;

	body_.friction(FRICTION);
	body_.gravity();
	body_.update(dt);
	body_.collide(plane);
}
