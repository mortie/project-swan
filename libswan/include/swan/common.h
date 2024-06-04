#pragma once

// IWYU pragma: begin_exports
#include <swan-common/trace.h>
#include <swan-common/Vector2.h>
#include <swan-common/constants.h>
// IWYU pragma: end_exports
#include <limits.h>
#include <stdint.h>
#include <optional>

// Forward declare the Cygnet::Renderer, because lots of functions will need
// to take a reference to it. It's nicer to not have to include Cygnet::Renderer
// in every header.
namespace Cygnet {

class Renderer;
class TextCache;

}

namespace Swan {

using namespace SwanCommon;

using TilePos = Vec2i;
using ChunkPos = Vec2i;
using ChunkRelPos = Vec2i;

inline ChunkPos tilePosToChunkPos(TilePos pos)
{
	// This might look weird, but it reduces an otherwise complex series of operations
	// including conditional branches into like four x64 instructions.
	// Basically, the problem is that we want 'floor(pos.x / CHUNK_WIDTH)', but
	// integer division rounds towards zero, it doesn't round down.
	// Solution: Move the position far to the right in the number line, do the math there
	// with integer division which always rounds down (because the numbers are always >0),
	// then move the result back to something hovering around 0 again.
	return ChunkPos(
		((long long)pos.x + (LLONG_MAX / 2) + 1) / CHUNK_WIDTH - ((LLONG_MAX / 2) / CHUNK_WIDTH) - 1,
		((long long)pos.y + (LLONG_MAX / 2) + 1) / CHUNK_HEIGHT - ((LLONG_MAX / 2) / CHUNK_HEIGHT) - 1);
}

inline ChunkRelPos tilePosToChunkRelPos(TilePos pos)
{
	// This uses a similar trick to chunkPos to turn a mess of conditional moves
	// and math instructions into literally one movabs and one 'and'
	return ChunkRelPos(
		(pos.x + (long long)CHUNK_WIDTH * ((LLONG_MAX / 2) / CHUNK_WIDTH)) % CHUNK_WIDTH,
		(pos.y + (long long)CHUNK_HEIGHT * ((LLONG_MAX / 2) / CHUNK_HEIGHT)) % CHUNK_HEIGHT);
}

class Game;
class World;
class WorldPlane;

struct Context {
	Game &game;
	World &world;
	WorldPlane &plane;
};

class Direction {
public:
	enum class Value: uint8_t {
		UP = 0,
		DOWN = 1,
		LEFT = 2,
		RIGHT = 3,
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
	constexpr Vector2<T> vec() {
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

	constexpr int asInt() {
		return (int)value_;
	}

	static constexpr std::optional<Direction> fromInt(int num)
	{
		if (num < 0 || num > 3) {
			return std::nullopt;
		}

		return Value(num);
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
