#include "Body.h"

namespace Swan {

void Body::friction(float coef) {
	force_ += -vel_ * coef;
}

void Body::gravity(Vec2 g) {
	force_ += g * mass_;
}

void Body::collide(WorldPlane &plane) {
	int startx = (int)pos_.x;
	int endx = (int)(pos_.x + size_.x);

	int y = (int)(pos_.y + size_.y);
	for (int x = startx; x <= endx; ++x) {
		Tile &tile = plane.getTile(x, y);
		if (!tile.opts_.transparent_) {
			pos_.y = y - size_.y;
			vel_.y = 0;
		}
	}
}

void Body::outline(Win &win) {
	sf::RectangleShape rect(size_ * TILE_SIZE);
	rect.setPosition(pos_ * TILE_SIZE);
	rect.setFillColor(sf::Color::Transparent);
	rect.setOutlineColor(sf::Color(128, 128, 128));
	rect.setOutlineThickness(2);
	win.draw(rect);
}

void Body::update(float dt) {
	vel_ += (force_ / mass_) * dt;
	pos_ += vel_ * dt;
	force_ = { 0, 0 };
}

}
