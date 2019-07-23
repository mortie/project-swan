#pragma once

#include <memory>

#include "Chunk.h"
#include "TileMap.h"
#include "WorldPlane.h"

namespace Swan {

class WorldGen {
public:
	class Factory {
	public:
		std::string name_;
		virtual WorldGen *create(TileMap &tmap) = 0;
		virtual ~Factory() = default;
	};

	virtual ~WorldGen() = default;

	virtual void genChunk(WorldPlane &plane, Chunk &chunk, int x, int y) = 0;
};

}
