#pragma once

#include "common.h"
#include "util.h"

namespace Swan {

class Direction {
public:
	enum class Value: uint8_t {
		UP = 1 << 0,
		DOWN = 1 << 1,
		LEFT = 1 << 2,
		RIGHT = 1 << 3,
	};

	using Value::UP;
	using Value::DOWN;
	using Value::LEFT;
	using Value::RIGHT;

	constexpr Direction(const Direction &) = default;
	constexpr Direction(Value v): value_(v) {}

	constexpr Direction &operator=(const Direction &) = default;

	constexpr operator Value() const
	{
		return value_;
	}

	explicit operator bool() = delete;

	constexpr Direction opposite() const
	{
		switch (*this) {
		case Direction::UP:
			return Direction::DOWN;
		case Direction::DOWN:
			return Direction::UP;
		case Direction::LEFT:
			return Direction::RIGHT;
		case Direction::RIGHT:
			return Direction::LEFT;
		}
	}

	template<typename T = int>
	constexpr Vector2<T> vec()
	{
		switch (*this) {
		case Direction::UP:
			return {0, -1};
		case Direction::DOWN:
			return {0, 1};
		case Direction::LEFT:
			return {-1, 0};
		case Direction::RIGHT:
			return {1, 0};
		}
	}

	constexpr int asInt()
	{
		return (int)value_;
	}

	static constexpr std::optional<Direction> fromInt(int num)
	{
		if (num < 0 || num > 3) {
			return std::nullopt;
		}

		return Value(num);
	}

	static Direction random()
	{
		int rand = ::Swan::random() % 4;
		return Value(1 << rand);
	}

private:
	Value value_;
};

inline constexpr TilePos operator+(TilePos pos, Direction dir)
{
	return pos + dir.vec();
}

inline constexpr TilePos operator+(Direction dir, TilePos pos)
{
	return dir.vec() + pos;
}

}
