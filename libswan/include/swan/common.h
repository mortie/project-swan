#pragma once

// We want every file to be able to easily add Tracy zones
#include <tracy/Tracy.hpp>

#include <swan-common/Vector2.h>
#include <swan-common/constants.h>

// Forward declare the Cygnet::Renderer, because lots of functions will need
// to take a reference to it. It's nicer to not have to include Cygnet::Renderer
// in every header.
namespace Cygnet {
class Renderer;
}

namespace Swan {

using namespace SwanCommon;

using TilePos = Vec2i;
using ChunkPos = Vec2i;
using ChunkRelPos = Vec2i;

inline ChunkPos tilePosToChunkPos(TilePos pos) {
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

inline ChunkRelPos tilePosToChunkRelPos(TilePos pos) {
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

}
