#include "Body.h"

#include <math.h>

#include "WorldPlane.h"

namespace Swan {

void Body::friction(Vec2 coef) {
	force_ += -vel_ * coef;
}

void Body::gravity(Vec2 g) {
	force_ += g * mass_;
}

void Body::collide(WorldPlane &plane) {
	int startx, endx, y;

	// This collission code is horrible and in dire need of some more abstractions.
	// I will fix later, ok? This works for now while working on more interesting things.

	// Collide with sides
	startx = (int)floor(pos_.x);
	endx = (int)floor(pos_.x + size_.x);
	y = (int)ceil(pos_.y + size_.y - 1.3);
	for (int x = startx; x <= endx; ++x) {
		for (int ry = y - 1; ry <= y; ++ry) {
			Tile &wall = plane.getTile(TilePos(x, ry));
			if (x == startx && vel_.x < 0 && wall.is_solid) {
				vel_.x = 0;
				pos_.x = startx + 1.001;
				startx = (int)floor(pos_.x);
			} else if (x == endx && vel_.x > 0 && wall.is_solid) {
				vel_.x = 0;
				pos_.x = endx - size_.x - 0.001;
				endx = (int)floor(pos_.x + size_.x);
			}
			//plane.debugBox(TilePos(x, ry));
		}
	}

	// Collide with top
	y = (int)ceil(pos_.y - 1);
	for (int x = startx; x <= endx; ++x) {
		Tile &roof = plane.getTile(TilePos(x, y));
		if (roof.is_solid && vel_.y < 0) {
			pos_.y = y + 1;
			vel_.y = 0;
		}

		//plane.debugBox(TilePos(x, y));
	}

	// Collide with floor
	on_ground_ = false;
	y = (int)ceil(pos_.y + size_.y - 1);
	for (int x = startx; x <= endx; ++x) {
		Tile &ground = plane.getTile(TilePos(x, y));
		if (ground.is_solid && vel_.y > 0) {
			pos_.y = y - size_.y;
			vel_.y = 0;
			on_ground_ = true;
		}
		//plane.debugBox(TilePos(x, y));
	}
}

void Body::outline(Win &win) {
	win.setPos(pos_);
	sf::RectangleShape rect(size_ * TILE_SIZE);
	rect.setFillColor(sf::Color::Transparent);
	rect.setOutlineColor(sf::Color(128, 128, 128));
	rect.setOutlineThickness(1);
	win.draw(rect);
}

void Body::update(float dt) {
	vel_ += (force_ / mass_) * dt;
	pos_ += vel_ * dt;
	force_ = { 0, 0 };
}

}
