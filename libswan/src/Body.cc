#include "Body.h"

namespace Swan {

void Body::friction(Vec2 coef) {
	force_ += -vel_ * coef;
}

void Body::gravity(Vec2 g) {
	force_ += g * mass_;
}

void Body::collide(WorldPlane &plane) {
	int px = (int)pos_.x_;
	if (pos_.x_ < 0) px -= 1;
	int startx = px;
	int endx = (int)(px + size_.x_);

	on_ground_ = false;
	int y = (int)pos_.y_ + size_.y_;
	for (int x = startx; x <= endx; ++x) {
		Tile &ground = plane.getTile(TilePos(x, y));
		if (ground.is_solid_ && vel_.y_ > 0) {
			pos_.y_ = y - size_.y_;
			vel_.y_ = 0;
			on_ground_ = true;
		}

		Tile &wall = plane.getTile(TilePos(x, y - 1));
		if (x == startx && vel_.x_ < 0) {
			if (wall.is_solid_) {
				vel_.x_ = 0;
				pos_.x_ = startx + 1;
			}
		} else if (x == endx && vel_.x_ > 0) {
			if (wall.is_solid_) {
				vel_.x_ = 0;
				pos_.x_ = endx - 1;
			}
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
