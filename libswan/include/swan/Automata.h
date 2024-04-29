#pragma once

#include <unordered_map>

#include "common.h"

namespace Cygnet {
class Renderer;
}

namespace Swan {

class Automata {
public:
	static constexpr int RESOLUTION = 3;

	using CellPos = Vec2i;

	enum class Cell: unsigned char {
		AIR,
		SOLID,
		WATER,
		WATER_L,
		WATER_R,
	};

	struct Window {
		Cell &self;
		Cell &left;
		Cell &right;
		Cell &above;
		Cell &below;
	};

	struct Chunk {
		Cell data[CHUNK_HEIGHT * RESOLUTION][CHUNK_WIDTH * RESOLUTION];
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
