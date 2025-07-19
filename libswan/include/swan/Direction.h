#pragma once

#include <ostream>

#include <swan/util.h>
#include "common.h"
#include "swan.capnp.h"

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
		switch (value_) {
		case Value::NONE:
			return Value::NONE;
		case Value::UP:
			return Value::DOWN;
		case Value::DOWN:
			return Value::UP;
		case Value::LEFT:
			return Value::RIGHT;
		case Value::RIGHT:
			return Value::LEFT;
		}

		return Value::NONE;
	}

	template<typename T = int>
	constexpr Vector2<T> vec()
	{
		switch (value_) {
		case Value::NONE:
			return {0, 0};
		case Value::UP:
			return {0, -1};
		case Value::DOWN:
			return {0, 1};
		case Value::LEFT:
			return {-1, 0};
		case Value::RIGHT:
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

	const char *name() {
		switch (value_) {
		case Value::NONE:
			return "None";
		case Value::UP:
			return "Up";
		case Value::DOWN:
			return "Down";
		case Value::LEFT:
			return "Left";
		case Value::RIGHT:
			return "Right";
		}

		return "Unknown";
	}

	void serialize(proto::Direction::Builder w)
	{
		w.setValue(asInt());
	}

	void deserialize(proto::Direction::Reader r)
	{
		auto val = fromInt(r.getValue());
		if (!val) {
			throw std::runtime_error("Invalid direction value");
		}

		value_ = val.value();
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
