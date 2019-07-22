#pragma once

#include <memory>

#include "Chunk.h"
#include "TileMap.h"

namespace Swan {

class WorldGen {
public:
	using ID = int;

	class Factory {
	public:
		std::string name_;
		Factory(const std::string &name): name_(name) {}
		virtual WorldGen *create(TileMap &tmap) = 0;
	};

	virtual ~WorldGen() = default;

	virtual void genChunk(Chunk &chunk, int x, int y) = 0;
};

}
