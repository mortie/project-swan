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

class Game;
class World;
class WorldPlane;

struct Context {
	Game &game;
	World &world;
	WorldPlane &plane;
};

}
