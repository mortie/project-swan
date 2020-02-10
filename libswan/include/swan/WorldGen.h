#pragma once

#include <memory>
#include <SDL.h>

#include "common.h"
#include "Chunk.h"
#include "Entity.h"
#include "traits/BodyTrait.h"
#include "Vector2.h"

namespace Swan {

class World;
class WorldPlane;
class ImageResource;

class WorldGen {
public:
	class Factory {
	public:
		virtual ~Factory() = default;
		virtual WorldGen *create(World &world) = 0;
		std::string name_;
	};

	virtual ~WorldGen() = default;

	virtual void drawBackground(const Context &ctx, Win &win, Vec2 pos) = 0;
	virtual SDL_Color backgroundColor(Vec2 pos) = 0;

	virtual void genChunk(WorldPlane &plane, Chunk &chunk) = 0;
	virtual BodyTrait::HasBody *spawnPlayer(WorldPlane &plane) = 0;
};

class WorldGenStructure {
public:
	virtual ~WorldGenStructure() = 0;

	virtual bool isBase(TilePos pos);
};

}
