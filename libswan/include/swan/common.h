#pragma once

#include "Vector2.h"

namespace Swan {

static constexpr int TILE_SIZE = 32;
static constexpr int TICK_RATE = 20;
static constexpr int CHUNK_HEIGHT = 64;
static constexpr int CHUNK_WIDTH = 64;
static constexpr int PLACEHOLDER_RED = 245;
static constexpr int PLACEHOLDER_GREEN = 66;
static constexpr int PLACEHOLDER_BLUE = 242;

using TilePos = Vec2i;
using ChunkPos = Vec2i;

class Game;
class World;
class WorldPlane;
class Win;
class ResourceManager;

struct Context {
	Game &game;
	World &world;
	WorldPlane &plane;
	ResourceManager &resources;
};

}
