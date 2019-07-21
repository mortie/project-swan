#pragma once

#include <vector>
#include <utility>

#include "common.h"
#include "Chunk.h"
#include "Tile.h"
#include "TileMap.h"

namespace Swan {

class World;

class WorldPlane {
public:
	using ID = uint16_t;
	using Coord = std::pair<int, int>;

	std::map<Coord, Chunk> chunks_;
	ID id_;
	World *world_;

	Chunk &getChunk(int x, int y);
	void setTileID(int x, int y, Tile::ID id);
	Tile &getTile(int x, int y);

	void draw(Win &win);
	void update(float dt);
	void tick();
};

}
