#pragma once

#include <iostream>
#include <cmath>
#include <array>

#include "Vector2.h"

namespace SwanCommon {

template<typename T>
struct Matrix3 {
	using Vec = Vector2<T>;

	static constexpr std::array<T, 9> identity = {1, 0, 0, 0, 1, 0, 0, 0, 1};
	std::array<T, 9> vals;

	constexpr Matrix3(): vals(identity) {}
	constexpr Matrix3(const Matrix3 &mat): vals(mat.vals) {}

	constexpr Matrix3 &operator=(const Matrix3 &mat) { vals = mat.vals; }

	constexpr T *data() {
		return vals.data();
	}

	constexpr const T *data() const {
		return vals.data();
	}

	constexpr T &at(int x, int y) {
		return vals[y * 3 + x];
	}

	constexpr const T &at(int x, int y) const {
		return vals[y * 3 + x];
	}

	constexpr Matrix3<T> &set(std::initializer_list<T> vals) {
		this->vals = vals;
		return *this;
	}

	constexpr Matrix3<T> &reset() {
		vals = identity;
		return *this;
	}

	constexpr Matrix3<T> &translate(Vec vec) {
		at(2, 0) += vec.x;
		at(2, 1) += vec.y;
		return *this;
	}

	constexpr Matrix3<T> &scale(Vec vec) {
		at(0, 0) *= vec.x;
		at(1, 1) *= vec.y;
		return *this;
	}

	constexpr Matrix3<T> &rotate(T rads) {
		T s = std::sin(rads);
		T c = std::cos(rads);
		at(0, 0) += c;
		at(1, 0) -= s;
		at(0, 1) += s;
		at(1, 1) += c;
		return *this;
	}

	static const Matrix3<T> IDENTITY;

	template<typename U>
	friend std::ostream &operator<<(std::ostream &os, const Matrix3<U> &mat);
};

template<typename T>
const Matrix3<T> Matrix3<T>::IDENTITY = Matrix3<T>();

template<typename T>
std::ostream &operator<<(std::ostream &os, const Matrix3<T> &mat) {
	os << '('
		<< '(' << mat.at(0, 0) << ", " << mat.at(1, 0) << ", " << mat.at(2, 0) << "), "
		<< '(' << mat.at(0, 1) << ", " << mat.at(1, 1) << ", " <<  mat.at(2, 1) << "), "
		<< '(' << mat.at(0, 2) << ", " << mat.at(1, 2) << ", " <<  mat.at(2, 2) << "))";
	return os;
}

}
