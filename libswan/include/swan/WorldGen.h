#pragma once

#include <memory>
#include <cygnet/util.h>

#include "common.h"
#include "Chunk.h"
#include "Entity.h"
#include "EntityCollection.h"

namespace Swan {

class World;
class WorldPlane;

class WorldGen {
public:
	struct Factory {
		const std::string name;
		std::unique_ptr<WorldGen> (*const create)(World & world);
	};

	virtual ~WorldGen() = default;

	virtual void drawBackground(Ctx &ctx, Cygnet::Renderer &rnd, Vec2 pos) = 0;
	virtual Cygnet::Color backgroundColor(Vec2 pos) = 0;

	virtual void genChunk(WorldPlane &plane, Chunk &chunk) = 0;
	virtual EntityRef spawnPlayer(Ctx &ctx) = 0;
};

}
