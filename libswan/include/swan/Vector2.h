#pragma once

#include <utility>

namespace Swan {

template<typename T>
struct Vector2 {
	T x;
	T y;

	constexpr Vector2(T x = 0, T y = 0): x(x), y(y) {}

	constexpr Vector2<T> &set(T x, T y) {
		this->x = x;
		this->y = y;
		return *this;
	}

	constexpr operator std::pair<T, T>() const {
		return std::pair<T, T>(x, y);
	}

	constexpr operator Vector2<float>() const {
		return Vector2<float>(x, y);
	}

	constexpr bool operator==(const Vector2<T> &vec) const {
		return x == vec.x && y == vec.y;
	}

	constexpr bool operator!=(const Vector2<T> &vec) const {
		return !(*this == vec);
	}

	constexpr Vector2<T> operator-() const {
		return Vector2<T>(-x, -y);
	}

	constexpr Vector2<T> operator+(const Vector2<T> &vec) const {
		return Vector2<T>(x + vec.x, y + vec.y);
	}
	constexpr Vector2<T> &operator+=(const Vector2<T> &vec) {
		x += vec.x;
		y += vec.y;
		return *this;
	}

	constexpr Vector2<T> operator-(const Vector2<T> &vec) const {
		return Vector2<T>(x - vec.x, y - vec.y);
	}
	constexpr Vector2<T> &operator-=(const Vector2<T> &vec) {
		x -= vec.x;
		y -= vec.y;
		return *this;
	}

	constexpr Vector2<T> operator*(const Vector2<T> &vec) const {
		return Vector2<T>(x * vec.x, y * vec.y);
	}
	constexpr Vector2<T> &operator*=(const Vector2<T> &vec) {
		x *= vec.x;
		y *= vec.y;
		return *this;
	}

	constexpr Vector2<T> operator*(T num) const {
		return Vector2<T>(x * num, y * num);
	}
	constexpr Vector2<T> operator*=(T num) {
		x *= num;
		y *= num;
		return *this;
	}

	constexpr Vector2<T> operator/(const Vector2<T> &vec) const {
		return Vector2<T>(x / vec.x, y / vec.y);
	}
	constexpr Vector2<T> &operator/=(const Vector2<T> &vec) {
		x /= vec.x;
		y /= vec.y;
		return *this;
	}

	constexpr Vector2<T> operator/(T num) const {
		return Vector2<T>(x / num, y / num);
	}
	constexpr Vector2<T> operator/=(T num) {
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
