#pragma once

// We want every file to be able to easily add Tracy zones
#include <tracy/Tracy.hpp>

#include <swan-common/Vector2.h>
#include <swan-common/constants.h>

namespace Swan {

using namespace SwanCommon;

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
