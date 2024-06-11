#pragma once

#include "common.h"
#include "util.h"

#include <ostream>

namespace Swan {

class Direction {
public:
	enum class Value: uint8_t {
		NONE = 0,
		UP = 1 << 0,
		DOWN = 1 << 1,
		LEFT = 1 << 2,
		RIGHT = 1 << 3,
	};

	using Value::NONE;
	using Value::UP;
	using Value::DOWN;
	using Value::LEFT;
	using Value::RIGHT;

	constexpr Direction() = default;
	constexpr Direction(const Direction &) = default;
	constexpr Direction(Value v): value_(v) {}

	constexpr Direction &operator=(const Direction &) = default;

	constexpr operator Value() const
	{
		return value_;
	}

	explicit operator bool() const
	{
		return *this != NONE;
	}

	constexpr Direction opposite() const
	{
		switch (*this) {
		case Direction::NONE:
			return Direction::NONE;
		case Direction::UP:
			return Direction::DOWN;
		case Direction::DOWN:
			return Direction::UP;
		case Direction::LEFT:
			return Direction::RIGHT;
		case Direction::RIGHT:
			return Direction::LEFT;
		}

		return Direction::NONE;
	}

	template<typename T = int>
	constexpr Vector2<T> vec()
	{
		switch (*this) {
		case Direction::NONE:
			return {0, 0};
		case Direction::UP:
			return {0, -1};
		case Direction::DOWN:
			return {0, 1};
		case Direction::LEFT:
			return {-1, 0};
		case Direction::RIGHT:
			return {1, 0};
		}

		return {};
	}

	constexpr int asInt()
	{
		return (int)value_;
	}

	static constexpr std::optional<Direction> fromInt(int num)
	{
		switch (num) {
		case (int)Value::NONE:
			return Value::NONE;
		case (int)Value::UP:
			return Value::UP;
		case (int)Value::DOWN:
			return Value::DOWN;
		case (int)Value::LEFT:
			return Value::LEFT;
		case (int)Value::RIGHT:
			return Value::RIGHT;
		}

		return std::nullopt;
	}

	static Direction random()
	{
		int rand = ::Swan::random() % 4;
		return Value(1 << rand);
	}

private:
	Value value_ = Value::NONE;
};

inline constexpr TilePos operator+(TilePos pos, Direction dir)
{
	return pos + dir.vec();
}

inline constexpr TilePos operator-(TilePos pos, Direction dir)
{
	return pos - dir.vec();
}

inline constexpr TilePos operator+(Direction dir, TilePos pos)
{
	return dir.vec() + pos;
}

inline std::ostream &operator<<(std::ostream &os, const Direction &dir)
{
	switch (dir) {
	case Direction::NONE:
		os << "Direction::NONE";
		break;
	case Direction::UP:
		os << "Direction::UP";
		break;
	case Direction::DOWN:
		os << "Direction::DOWN";
		break;
	case Direction::LEFT:
		os << "Direction::LEFT";
		break;
	case Direction::RIGHT:
		os << "Direction::RIGHT";
		break;
	}

	return os;
}

}
