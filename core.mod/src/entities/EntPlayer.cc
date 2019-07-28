#include "EntPlayer.h"

EntPlayer::EntPlayer(const Swan::Vec2 &pos):
		body_(pos, SIZE, MASS) {
	texture_.create(animation_still_.width_, animation_still_.height_);
	sprite_ = sf::Sprite(texture_);
}

void EntPlayer::draw(Swan::Win &win) {
	body_.outline(win);

	if (animation_still_.fill(texture_))
		sprite_.setTexture(texture_);

	win.setPos(body_.pos_);
	win.draw(sprite_);
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

	animation_still_.tick(dt);
}
