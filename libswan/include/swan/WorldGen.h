#pragma once

#include <memory>
#include <SDL.h>

#include "common.h"
#include "Chunk.h"
#include "Entity.h"
#include "Collection.h"
#include "traits/BodyTrait.h"

namespace Swan {

class World;
class WorldPlane;
class ImageResource;

class WorldGen {
public:
	struct Factory {
		const std::string name;
		std::unique_ptr<WorldGen> (*const create)(World &world);
	};

	virtual ~WorldGen() = default;

	virtual void drawBackground(const Context &ctx, Win &win, Vec2 pos) = 0;
	virtual SDL_Color backgroundColor(Vec2 pos) = 0;

	virtual void genChunk(WorldPlane &plane, Chunk &chunk) = 0;
	virtual EntityRef spawnPlayer(const Context &ctx) = 0;
};

}
