#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>

#include "Vector2.h"

namespace Swan {

static constexpr float TILE_SIZE = 32;
static constexpr int TICK_RATE = 20;
static constexpr int CHUNK_HEIGHT = 32;
static constexpr int CHUNK_WIDTH = 32;

struct Win {
public:
	sf::RenderWindow *window_;
	sf::Transform transform_;
	Vec2 cam_;

	Win(sf::RenderWindow *win): window_(win) {}

	void setPos(const Vec2 &pos) {
		transform_ = sf::Transform().translate(pos - cam_);
	}

	void draw(const sf::Drawable &drawable) {
		window_->draw(drawable, transform_);
	}
};

}
