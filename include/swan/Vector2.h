#pragma once

#include <utility>
#include <ostream>
#include <cmath>
#include <type_traits>

namespace Swan {

template<typename T>
struct Vector2 {
	T x;
	T y;

	constexpr Vector2(): x(0), y(0)
	{}

	constexpr Vector2(T x, T y): x(x), y(y)
	{}

	constexpr Vector2<T> &set(T x, T y)
	{
		this->x = x;
		this->y = y;
		return *this;
	}

	template<typename U>
	constexpr Vector2<U> as() const
	{
		if constexpr (std::is_integral_v<U> && std::is_floating_point_v<T>) {
			return {
				U(std::floor(x)),
				U(std::floor(y)),
			};
		} else {
			return {U(x), U(y)};
		}
	}

	constexpr T length() const
	{
		return (T)std::sqrt((double)squareLength());
	}

	constexpr T squareLength() const
	{
		return this->x * this->x + this->y * this->y;
	}

	constexpr Vector2<T> sign() const
	{
		return Vector2<T>(x > 0 ? 1 : -1, y > 0 ? 1 : -1);
	}

	constexpr Vector2<T> add(T ax, T ay) const
	{
		return {x + ax, y + ay};
	}

	constexpr Vector2<T> scale(T sx, T sy) const
	{
		return Vector2<T>(x * sx, y * sy);
	}

	constexpr Vector2<T> scale(T s) const
	{
		return scale(s, s);
	}

	constexpr Vector2<T> norm() const
	{
		return *this / length();
	}

	constexpr T dot(const Vector2<T> &vec) const
	{
		return x * vec.x + y * vec.y;
	}

	constexpr operator Vector2<float>() const
	{
		return Vector2<float>(x, y);
	}

	constexpr bool operator==(const Vector2<T> &vec) const
	{
		return x == vec.x && y == vec.y;
	}

	constexpr bool operator!=(const Vector2<T> &vec) const
	{
		return !(*this == vec);
	}

	constexpr Vector2<T> operator-() const
	{
		return Vector2<T>(-x, -y);
	}

	constexpr Vector2<T> operator+(const Vector2<T> &vec) const
	{
		return Vector2<T>(x + vec.x, y + vec.y);
	}

	constexpr Vector2<T> &operator+=(const Vector2<T> &vec)
	{
		x += vec.x;
		y += vec.y;
		return *this;
	}

	constexpr Vector2<T> operator-(const Vector2<T> &vec) const
	{
		return Vector2<T>(x - vec.x, y - vec.y);
	}

	constexpr Vector2<T> &operator-=(const Vector2<T> &vec)
	{
		x -= vec.x;
		y -= vec.y;
		return *this;
	}

	constexpr Vector2<T> operator*(const Vector2<T> &vec) const
	{
		return Vector2<T>(x * vec.x, y * vec.y);
	}

	constexpr Vector2<T> &operator*=(const Vector2<T> &vec)
	{
		x *= vec.x;
		y *= vec.y;
		return *this;
	}

	constexpr Vector2<T> operator*(T num) const
	{
		return Vector2<T>(x * num, y * num);
	}

	constexpr Vector2<T> operator*=(T num)
	{
		x *= num;
		y *= num;
		return *this;
	}

	constexpr Vector2<T> operator/(const Vector2<T> &vec) const
	{
		return Vector2<T>(x / vec.x, y / vec.y);
	}

	constexpr Vector2<T> &operator/=(const Vector2<T> &vec)
	{
		x /= vec.x;
		y /= vec.y;
		return *this;
	}

	constexpr Vector2<T> operator/(T num) const
	{
		return Vector2<T>(x / num, y / num);
	}

	constexpr Vector2<T> operator/=(T num)
	{
		x /= num;
		y /= num;
		return *this;
	}

	static const Vector2<T> ZERO;

	template<typename U>
	friend std::ostream &operator<<(std::ostream &os, const Vector2<U> &vec);
};

template<typename T>
const Vector2<T> Vector2<T>::ZERO = Vector2<T>(0, 0);

template<typename T>
std::ostream &operator<<(std::ostream &os, const Vector2<T> &vec)
{
	os << '(' << vec.x << ", " << vec.y << ')';
	return os;
}

using Vec2 = Vector2<float>;
using Vec2i = Vector2<int>;

}

namespace std {

template<typename T>
struct hash<Swan::Vector2<T>> {
	std::size_t operator()(const Swan::Vector2<T> &vec) const
	{
		std::size_t x = vec.x, y = vec.y;
		return std::hash<std::size_t>{}(((x + y) * (x + y + 1) / 2) + y);
	}
};

}
