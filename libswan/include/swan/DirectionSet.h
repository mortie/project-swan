#pragma once

#include <assert.h>
#include <bit>
#include <optional>

#include "Direction.h"
#include "util.h"

namespace Swan {

class DirectionSet {
public:
	class Iterator;

	constexpr DirectionSet() = default;
	constexpr DirectionSet(const DirectionSet &) = default;
	constexpr DirectionSet(Direction dir):
		value_(dir.asInt())
	{}

	constexpr DirectionSet &operator=(const DirectionSet &) = default;

	static constexpr DirectionSet all()
	{
		DirectionSet dirs;
		dirs.value_ |= 0x0f;
		return dirs;
	}

	constexpr operator bool() const
	{
		return value_;
	}

	constexpr DirectionSet &operator|=(Direction b)
	{
		value_ |= b.asInt();
		return *this;
	}

	constexpr bool has(Direction dir)
	{
		return value_ & dir.asInt();
	}

	constexpr void set(Direction dir)
	{
		*this |= dir;
	}

	constexpr void unset(Direction dir)
	{
		value_ &= ~dir.asInt();
	}

	std::optional<Direction> random()
	{
		if (!*this) {
			return std::nullopt;
		}

		int choice = ::Swan::random() % 4;
		while (!(value_ & (1 << choice))) {
			choice = (choice + 1) % 4;
		}

		return Direction::fromInt(1 << choice).value();
	}

	constexpr Iterator begin() const;
	constexpr Iterator end() const;

private:
	uint8_t value_ = 0;
};

class DirectionSet::Iterator {
public:
	constexpr Iterator() = default;
	constexpr Iterator(const Iterator &other) = default;
	constexpr Iterator(uint8_t val): value_(val) {}

	friend constexpr bool operator==(Iterator a, Iterator b) = default;
	constexpr Iterator &operator=(const Iterator &other) = default;

	constexpr Direction operator*()
	{
		assert(value_ != 0);

		unsigned char val = std::countr_zero((unsigned char)value_);
		return Direction::fromInt(1 << val).value();
	}

	constexpr Iterator &operator++()
	{
		assert(value_ != 0);

		unsigned char val = std::countr_zero((unsigned char)value_);
		value_ &= ~(1 << val);
		return *this;
	}

private:
	uint8_t value_ = 0;
};

inline constexpr DirectionSet::Iterator DirectionSet::begin() const
{
	return {value_};
}

inline constexpr DirectionSet::Iterator DirectionSet::end() const
{
	return {0};
}

constexpr DirectionSet operator|(Direction a, Direction b)
{
	DirectionSet dirs;
	dirs |= a;
	dirs |= b;
	return dirs;
}

constexpr DirectionSet operator|(Direction::Value a, Direction::Value b)
{
	DirectionSet dirs;
	dirs |= a;
	dirs |= b;
	return dirs;
}

constexpr DirectionSet operator|(DirectionSet a, Direction b)
{
	DirectionSet res = a;
	res |= b;
	return res;
}

}
