#pragma once

#include <memory>

#include "Chunk.h"
#include "TileMap.h"

namespace Swan {

class WorldGen {
public:
	class Factory {
	public:
		std::string name_;
		virtual WorldGen *create(TileMap &tmap) = 0;
	};

	virtual ~WorldGen() = default;

	virtual void genChunk(Chunk &chunk, int x, int y) = 0;
};

}
