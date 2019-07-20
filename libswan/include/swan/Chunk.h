#pragma once

#include "common.h"
#include "Tile.h"

namespace Swan {

class Chunk {
public:
	int x_;
	int y_;
	Tile::TileID tiles_[CHUNK_WIDTH][CHUNK_HEIGHT];

	void setTile(int x, int y, Tile::TileID tile) {
		tiles_[x][y] = tile;
	}

	void clear();
	void draw(Win &win);
};

}
