#pragma once

#include <vector>

#include "common.h"
#include "Chunk.h"
#include "Tile.h"

namespace Swan {

class WorldPlane {
public:
	using PlaneID = uint16_t;

	std::vector<Chunk> chunks_;
	PlaneID id_;

	void setTile(int x, int y, Tile::TileID tile);

	void draw(Win &win);
	void update(float dt);
	void tick();
};

}
