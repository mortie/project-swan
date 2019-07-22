#pragma once

#include <vector>
#include <utility>
#include <memory>

#include "common.h"
#include "Chunk.h"
#include "Tile.h"
#include "TileMap.h"
#include "WorldGen.h"

namespace Swan {

class World;

class WorldPlane {
public:
	using ID = uint16_t;

	std::map<Chunk::ChunkPos, Chunk> chunks_;
	ID id_;
	World *world_;
	std::shared_ptr<WorldGen> gen_;

	WorldPlane(ID id, World *world, std::shared_ptr<WorldGen> gen):
		id_(id), world_(world), gen_(gen) {}

	Chunk &getChunk(int x, int y);
	void setTileID(int x, int y, Tile::ID id);
	Tile &getTile(int x, int y);

	void draw(Win &win);
	void update(float dt);
	void tick();
};

}
