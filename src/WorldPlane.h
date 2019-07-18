#pragma once

#include <vector>

#include "common.h"
#include "Chunk.h"

class WorldPlane {
public:
	std::vector<Chunk> chunks_;
	int max_chunk_x_ = 0;
	int min_chunk_x_ = 0;
	int max_chunk_y_ = 0;
	int min_chunk_y_ = 0;

	void draw(Win &win);
	void update(float dt);
	void tick();
};
