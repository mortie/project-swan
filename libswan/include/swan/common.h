#pragma once

// IWYU pragma: begin_exports
#include <bitset>
#include <cmath>
#include <swan/trace.h>
#include <swan/Vector2.h>
#include <swan/HashMap.h>
#include <swan/constants.h>
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
class Gui;

}

namespace Swan {

/// The world position of a tile.
struct TilePos: Vec2i {
	using Vec2i::Vec2i;
	constexpr TilePos(Vec2i vec): Vec2i(vec) {}
};

// The world position of a fluid tile.
struct FluidPos: Vec2i64  {
	using Vec2i64::Vec2i64;
	constexpr FluidPos(Vec2i64 vec): Vec2i64(vec) {}
};

/// The position of a chunk.
struct ChunkPos: Vec2i {
	using Vec2i::Vec2i;
	constexpr ChunkPos(Vec2i vec): Vec2i(vec) {}
};

/// The relative position of a tile within a chunk.
struct ChunkRelPos: Vec2i {
	using Vec2i::Vec2i;
	constexpr ChunkRelPos(Vec2i vec): Vec2i(vec) {}
};

using FluidCollision = std::bitset<FLUID_RESOLUTION * FLUID_RESOLUTION>;

inline constexpr ChunkPos chunkPos(TilePos pos)
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

inline constexpr ChunkRelPos chunkRelPos(TilePos pos)
{
	// This uses a similar trick to chunkPos to turn a mess of conditional moves
	// and math instructions into literally one movabs and one 'and'
	return ChunkRelPos(
		(pos.x + (long long)CHUNK_WIDTH * ((LLONG_MAX / 2) / CHUNK_WIDTH)) % CHUNK_WIDTH,
		(pos.y + (long long)CHUNK_HEIGHT * ((LLONG_MAX / 2) / CHUNK_HEIGHT)) % CHUNK_HEIGHT);
}

inline constexpr TilePos tilePos(Vec2 pos)
{
	return {
		(int32_t)round(pos.x),
		(int32_t)round(pos.y),
	};
}

inline constexpr Vec2 tileCenter(TilePos pos)
{
	return {
		float(pos.x) + 0.5f,
		float(pos.y) + 0.5f,
	};
}

class Game;
class World;
class WorldPlane;

struct Context {
	Game &game;
	World &world;
	WorldPlane &plane;
	Cygnet::Gui &gui;
};
using Ctx = const Context;

}
