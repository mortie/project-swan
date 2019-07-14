#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics.hpp>

#define UNIT_SIZE 32.0

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
	sf::RenderWindow &window_;
	sf::Transform &transform_;
	Vec2 curr_pos_ = { 0, 0 };

	void setPos(Vec2 &pos) {
		transform_.translate(-curr_pos_.x, -curr_pos_.y);
		transform_.translate(pos.x, pos.y);
		curr_pos_ = pos;
	}

	void draw(const sf::Drawable &drawable) {
		window_.draw(drawable, transform_);
	}
};
