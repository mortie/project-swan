#include "Body.h"

namespace Swan {

void Body::friction(Vec2 coef) {
	force_ += -vel_ * coef;
}

void Body::gravity(Vec2 g) {
	force_ += g * mass_;
}

void Body::collide(WorldPlane &plane) {
	int startx = (int)pos_.x_;
	int endx = (int)(pos_.x_ + size_.x_);

	int y = (int)(pos_.y_ + size_.y_);
	on_ground_ = false;
	for (int x = startx; x <= endx; ++x) {
		Tile &tile = plane.getTile(TilePos(x, y));
		if (tile.is_solid_) {
			pos_.y_ = y - size_.y_;
			vel_.y_ = 0;
			on_ground_ = true;
			break;
		}
	}
}

void Body::outline(Win &win) {
	win.setPos(pos_ * TILE_SIZE);
	sf::RectangleShape rect(size_ * TILE_SIZE);
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
