#pragma once

#include <SFML/System/Vector2.hpp>

namespace Swan {

template<typename T>
class Vector2 {
public:
	T x_;
	T y_;

	constexpr Vector2(T x = 0, T y = 0): x_(x), y_(y) {}

	operator sf::Vector2<T>() const {
		return sf::Vector2<T>(x_, y_);
	}

	Vector2<T> operator-() const {
		return Vector2<T>(-x_, -y_);
	}

	Vector2<T> operator+(const Vector2<T> &vec) const {
		return Vector2<T>(x_ + vec.x_, y_ + vec.y_);
	}
	Vector2<T> &operator+=(const Vector2<T> &vec) {
		x_ += vec.x_;
		y_ += vec.y_;
		return *this;
	}

	Vector2<T> operator-(const Vector2<T> &vec) const {
		return Vector2<T>(x_ - vec.x_, y_ - vec.y_);
	}
	Vector2<T> &operator-=(const Vector2<T> &vec) {
		x_ -= vec.x_;
		y_ -= vec.y_;
		return *this;
	}

	Vector2<T> operator*(const Vector2<T> &vec) const {
		return Vector2<T>(x_ * vec.x_, y_ * vec.y_);
	}
	Vector2<T> &operator*=(const Vector2<T> &vec) {
		x_ *= vec.x_;
		y_ *= vec.y_;
		return *this;
	}

	Vector2<T> operator*(T num) const {
		return Vector2<T>(x_ * num, y_ * num);
	}
	Vector2<T> operator*=(T num) {
		x_ *= num;
		y_ *= num;
		return *this;
	}

	Vector2<T> operator/(const Vector2<T> &vec) const {
		return Vector2<T>(x_ / vec.x_, y_ / vec.y_);
	}
	Vector2<T> &operator/=(const Vector2<T> &vec) {
		x_ /= vec.x_;
		y_ /= vec.y_;
		return *this;
	}

	Vector2<T> operator/(T num) const {
		return Vector2<T>(x_ / num, y_ / num);
	}
	Vector2<T> operator/=(T num) {
		x_ /= num;
		y_ /= num;
		return *this;
	}

	bool operator<(const Vector2<T> &vec) const {
		if (x_ < vec.x_) return true;
		if (vec.x_ < x_) return false;
		return y_ < vec.y_;
	}
};

using Vec2 = Vector2<float>;

}
