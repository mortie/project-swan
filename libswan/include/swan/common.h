#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>

#include "Vector2.h"

namespace Swan {

static constexpr int TILE_SIZE = 32;
static constexpr int TICK_RATE = 20;
static constexpr int CHUNK_HEIGHT = 16;
static constexpr int CHUNK_WIDTH = 24;

using TilePos = Vec2i;
using ChunkPos = Vec2i;

class WorldPlane;

struct Win {
public:
	sf::RenderWindow *window_;
	sf::Transform transform_;
	Vec2 cam_;
	double scale_ = 2;

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
		sf::Vector2 v = window_->getSize();
		return Vec2(v.x, v.y) / (TILE_SIZE * scale_);
	}
};

}
