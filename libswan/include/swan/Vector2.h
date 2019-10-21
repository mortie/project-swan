#pragma once

#include <SFML/System/Vector2.hpp>
#include <utility>

namespace Swan {

template<typename T>
struct Vector2 {
	T x;
	T y;

	constexpr Vector2(T x = 0, T y = 0): x(x), y(y) {}

	Vector2<T> &set(T x, T y) {
		this->x = x;
		this->y = y;
		return *this;
	}

	operator sf::Vector2<T>() const {
		return sf::Vector2<T>(x, y);
	}

	operator std::pair<T, T>() const {
		return std::pair<T, T>(x, y);
	}

	operator Vector2<float>() const {
		return Vector2<float>(x, y);
	}

	bool operator==(const Vector2<T> &vec) const {
		return x == vec.x && y == vec.y;
	}

	bool operator!=(const Vector2<T> &vec) const {
		return !(*this == vec);
	}

	Vector2<T> operator-() const {
		return Vector2<T>(-x, -y);
	}

	Vector2<T> operator+(const Vector2<T> &vec) const {
		return Vector2<T>(x + vec.x, y + vec.y);
	}
	Vector2<T> &operator+=(const Vector2<T> &vec) {
		x += vec.x;
		y += vec.y;
		return *this;
	}

	Vector2<T> operator-(const Vector2<T> &vec) const {
		return Vector2<T>(x - vec.x, y - vec.y);
	}
	Vector2<T> &operator-=(const Vector2<T> &vec) {
		x -= vec.x;
		y -= vec.y;
		return *this;
	}

	Vector2<T> operator*(const Vector2<T> &vec) const {
		return Vector2<T>(x * vec.x, y * vec.y);
	}
	Vector2<T> &operator*=(const Vector2<T> &vec) {
		x *= vec.x;
		y *= vec.y;
		return *this;
	}

	Vector2<T> operator*(T num) const {
		return Vector2<T>(x * num, y * num);
	}
	Vector2<T> operator*=(T num) {
		x *= num;
		y *= num;
		return *this;
	}

	Vector2<T> operator/(const Vector2<T> &vec) const {
		return Vector2<T>(x / vec.x, y / vec.y);
	}
	Vector2<T> &operator/=(const Vector2<T> &vec) {
		x /= vec.x;
		y /= vec.y;
		return *this;
	}

	Vector2<T> operator/(T num) const {
		return Vector2<T>(x / num, y / num);
	}
	Vector2<T> operator/=(T num) {
		x /= num;
		y /= num;
		return *this;
	}

	static const Vector2<T> ZERO;
};

template<typename T> const Vector2<T> Vector2<T>::ZERO = Vector2<T>(0, 0);

using Vec2 = Vector2<float>;
using Vec2i = Vector2<int>;

}
