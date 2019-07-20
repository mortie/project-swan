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
	using ID = uint16_t;

	std::vector<Chunk> chunks_;
	ID id_;
	World *world_;

	void setTile(int x, int y, Tile::ID id);

	void draw(Win &win);
	void update(float dt);
	void tick();
};

}
