#pragma once

#include <vector>

#include "common.h"
#include "Chunk.h"
#include "Tile.h"
#include "TileMap.h"

namespace Swan {

class World;

class WorldPlane {
public:
	using PlaneID = uint16_t;

	std::vector<Chunk> chunks_;
	PlaneID id_;
	World *world_;

	void setTile(int x, int y, Tile::TileID id);

	void draw(Win &win);
	void update(float dt);
	void tick();
};

}
