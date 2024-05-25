#pragma once

#include <unordered_map>
#include <vector>

#include "common.h"

namespace Cygnet {

class Renderer;

}

namespace Swan {

class Automata {
public:
	static constexpr int RESOLUTION = 3;
	static constexpr int AM_CHUNK_WIDTH = CHUNK_WIDTH * RESOLUTION;
	static constexpr int AM_CHUNK_HEIGHT = CHUNK_WIDTH * RESOLUTION;

	using CellPos = Vec2i;

	enum class Cell: unsigned char {
		AIR = 0,
		SOLID = 1,
		WATER = 2,
		WATER_L = 3,
		WATER_R = 4,
	};

	struct Window {
		Cell &self;
		Cell &left;
		Cell &right;
		Cell &above;
		Cell &below;
	};

	struct Chunk {
		Cell data[AM_CHUNK_HEIGHT][AM_CHUNK_WIDTH];
		bool modified = false;
		bool nextModified = false;

		Cell *operator[](int y)
		{
			return data[y];
		}
	};

	void tick();

	void fill(TilePos pos)
	{
		setTile(pos, Cell::SOLID);
	}

	void clear(TilePos pos)
	{
		setTile(pos, Cell::AIR);
	}

	void fillWater(TilePos pos)
	{
		setTile(pos, Cell::WATER);
	}

	void draw(ChunkPos pos, Cygnet::Renderer &rnd);

private:
	struct MoveEvent {
		Cell *src;
		Cell *dest;

		Cell srcCell = *src;
		Cell destCell = *dest;
		Cell after = srcCell;
	};

	void setTile(TilePos pos, Cell value);
	Chunk &getChunk(ChunkPos cpos);

	void tickChunk(ChunkPos pos, Chunk &chunk);
	bool rule(Window win);

	std::unordered_map<ChunkPos, Chunk> chunks_;
	std::vector<MoveEvent> moveEvents_;
};

}
