#pragma once

#include <ostream>
#include <cmath>
#include <array>

#include "Vector2.h"

namespace Swan {

// 3D transformation matrix.
// All the operations assume that the last row remains {0, 0, 1}.
// If the last row is modified, methods like .scale, .translate and .rotate won't work.
template<typename T>
struct Matrix3 {
	using Vec = Vector2<T>;

	static constexpr std::array<T, 9> identity = {1, 0, 0, 0, 1, 0, 0, 0, 1};
	std::array<T, 9> vals;

	constexpr Matrix3(): vals(identity)
	{}
	constexpr Matrix3(std::array<T, 9> vals): vals(vals)
	{}
	constexpr Matrix3(const Matrix3 &mat): vals(mat.vals)
	{}

	constexpr Matrix3 &operator=(const Matrix3 &mat)
	{
		vals = mat.vals;
		return *this;
	}

	constexpr T *data()
	{
		return vals.data();
	}

	constexpr const T *data() const
	{
		return vals.data();
	}

	constexpr T &at(int x, int y)
	{
		return vals[y * 3 + x];
	}

	constexpr const T &at(int x, int y) const
	{
		return vals[y * 3 + x];
	}

	constexpr T &operator()(int x, int y)
	{
		return at(x, y);
	}

	constexpr const T &operator()(int x, int y) const
	{
		return at(x, y);
	}

	constexpr Matrix3<T> &set(std::initializer_list<T> vals)
	{
		this->vals = vals;
		return *this;
	}

	constexpr Matrix3<T> &reset()
	{
		vals = identity;
		return *this;
	}

	constexpr Matrix3<T> &translate(Vec vec)
	{
		at(2, 0) += vec.x;
		at(2, 1) += vec.y;
		return *this;
	}

	constexpr Matrix3<T> &scale(Vec vec)
	{
		at(0, 0) *= vec.x;
		at(1, 0) *= vec.x;
		at(2, 0) *= vec.x;
		at(0, 1) *= vec.y;
		at(1, 1) *= vec.y;
		at(2, 1) *= vec.y;
		return *this;
	}

	constexpr Matrix3<T> &rotate(T rads)
	{
		T s = std::sin(rads);
		T c = std::cos(rads);
		T old00 = at(0, 0), old10 = at(1, 0), old20 = at(2, 0);

		at(0, 0) = c * old00 + (-s * at(0, 1));
		at(1, 0) = c * old10 + (-s * at(1, 1));
		at(2, 0) = c * old20 + (-s * at(2, 1));
		at(0, 1) = s * old00 + c * at(0, 1);
		at(1, 1) = s * old10 + c * at(1, 1);
		at(2, 1) = s * old20 + c * at(2, 1);
		return *this;
	}

	constexpr friend Matrix3<T> operator*(const Matrix3<T> &a, const Matrix3<T> &b)
	{
		return Matrix3<T>({
			a(0, 0) * b(0, 0) + a(0, 1) * b(1, 0) + a(0, 2) * b(2, 0),
			a(0, 0) * b(0, 1) + a(0, 1) * b(1, 1) + a(0, 2) * b(2, 1),
			a(0, 0) * b(0, 2) + a(0, 1) * b(1, 2) + a(0, 2) * b(2, 2),

			a(1, 0) * b(0, 0) + a(1, 1) * b(1, 0) + a(1, 2) * b(2, 0),
			a(1, 0) * b(0, 1) + a(1, 1) * b(1, 1) + a(1, 2) * b(2, 1),
			a(1, 0) * b(0, 2) + a(1, 1) * b(1, 2) + a(1, 2) * b(2, 2),

			a(2, 0) * b(0, 0) + a(2, 1) * b(1, 0) + a(2, 2) * b(2, 0),
			a(2, 0) * b(0, 1) + a(2, 1) * b(1, 1) + a(2, 2) * b(2, 1),
			a(2, 0) * b(0, 2) + a(2, 1) * b(1, 2) + a(2, 2) * b(2, 2),
		});
	}

	constexpr Matrix3<T> operator*=(const Matrix3<T> &other)
	{
		*this = *this * other;
		return *this;
	}

	static const Matrix3<T> IDENTITY;

	template<typename U>
	friend std::ostream &operator<<(std::ostream &os, const Matrix3<U> &mat);
};

template<typename T>
const Matrix3<T> Matrix3<T>::IDENTITY = Matrix3<T>();

template<typename T>
std::ostream &operator<<(std::ostream &os, const Matrix3<T> &mat)
{
	os
		<< '('
		<< '(' << mat.at(0, 0) << ", " << mat.at(1, 0) << ", " << mat.at(2, 0) << "), "
		<< '(' << mat.at(0, 1) << ", " << mat.at(1, 1) << ", " << mat.at(2, 1) << "), "
		<< '(' << mat.at(0, 2) << ", " << mat.at(1, 2) << ", " << mat.at(2, 2) << "))";
	return os;
}

}
