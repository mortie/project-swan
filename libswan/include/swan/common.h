#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>

#include "Vector2.h"

namespace Swan {

static constexpr int TILE_SIZE = 32;
static constexpr int TICK_RATE = 20;
static constexpr int CHUNK_HEIGHT = 64;
static constexpr int CHUNK_WIDTH = 64;

using TilePos = Vec2i;
using ChunkPos = Vec2i;

class Game;
class World;
class WorldPlane;
class Win;

struct Context {
	Game &game;
	World &world;
	WorldPlane &plane;
};

}
