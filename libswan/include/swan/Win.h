#pragma once

#include "common.h"

namespace Swan {

class Win {
public:
	float scale_ = 2;
	Vec2 cam_;

	Win(sf::RenderWindow *win): window_(win) {}

	void setPos(const Vec2 &pos) {
		transform_ = sf::Transform()
			.scale(scale_, scale_)
			.translate((pos - cam_) * TILE_SIZE);
	}

	void draw(const sf::Drawable &drawable) {
		window_->draw(drawable, transform_);
	}

	Vec2 getSize() {
		sf::Vector2u v = window_->getSize();
		return Vec2(v.x, v.y) / (TILE_SIZE * scale_);
	}

private:
	sf::RenderWindow *window_;
	sf::Transform transform_;
};

}
