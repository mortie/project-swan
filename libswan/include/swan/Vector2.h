#pragma once

#include <SFML/System/Vector2.hpp>
#include <utility>

namespace Swan {

template<typename T>
class Vector2 {
public:
	constexpr Vector2(T x = 0, T y = 0): x_(x), y_(y) {}

	operator sf::Vector2<T>() const {
		return sf::Vector2<T>(x_, y_);
	}

	operator std::pair<T, T>() const {
		return std::pair<T, T>(x_, y_);
	}

	bool operator==(const Vector2<T> &vec) const {
		return x_ == vec.x_ && y_ == vec.y_;
	}

	bool operator!=(const Vector2<T> &vec) const {
		return !(*this == vec);
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

	T x_;
	T y_;

	static constexpr Vector2<T> ZERO = Vector2<T>(0, 0);
};

using Vec2 = Vector2<float>;
using Vec2i = Vector2<int>;

}
