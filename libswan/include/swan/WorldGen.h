#pragma once

#include <memory>

#include "Chunk.h"
#include "TileMap.h"

namespace Swan {

class WorldPlane;

class WorldGen {
public:
	class Factory {
	public:
		virtual ~Factory() = default;
		virtual WorldGen *create(TileMap &tmap) = 0;
		std::string name_;
	};

	virtual ~WorldGen() = default;

	virtual void genChunk(WorldPlane &plane, Chunk &chunk, int x, int y) = 0;
};

}
