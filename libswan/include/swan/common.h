#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics.hpp>

namespace Swan {

static constexpr float TILE_SIZE = 32;
static constexpr int TICK_RATE = 20;
static constexpr int CHUNK_HEIGHT = 32;
static constexpr int CHUNK_WIDTH = 32;

class Vec2: public sf::Vector2<float> {
public:
	using sf::Vector2<float>::Vector2;

	Vec2 operator+(Vec2 &vec) {
		return Vec2(x + vec.x, y + vec.y);
	}
	Vec2 &operator+=(Vec2 &vec) {
		this->x += vec.x;
		this->y += vec.y;
		return *this;
	}

	Vec2 operator-(Vec2 &vec) {
		return Vec2(x - vec.x, y - vec.y);
	}
	Vec2 &operator-=(Vec2 &vec) {
		this->x -= vec.x;
		this->y -= vec.y;
		return *this;
	}

	Vec2 operator*(Vec2 &vec) {
		return Vec2(x * vec.x, y * vec.y);
	}
	Vec2 &operator*=(Vec2 &vec) {
		this->x *= vec.x;
		this->y *= vec.y;
		return *this;
	}

	Vec2 operator/(Vec2 &vec) {
		return Vec2(x / vec.x, y / vec.y);
	}
	Vec2 &operator/=(Vec2 &vec) {
		this->x /= vec.x;
		this->y /= vec.y;
		return *this;
	}
};

struct Win {
public:
	sf::RenderWindow *window_;
	sf::Transform transform_;

	Win(sf::RenderWindow *win): window_(win) {}

	void setPos(const Vec2 &pos) {
		transform_ = sf::Transform().translate(pos);
	}

	void draw(const sf::Drawable &drawable) {
		window_->draw(drawable, transform_);
	}
};

}
