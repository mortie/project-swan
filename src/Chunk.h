#pragma once

#include "common.h"
#include "Tile.h"

class Chunk {
public:
	int x_;
	int y_;
	Tile::TileID tiles_[CHUNK_HEIGHT][CHUNK_WIDTH];

	void clear();
	void draw(Win &win);
};
