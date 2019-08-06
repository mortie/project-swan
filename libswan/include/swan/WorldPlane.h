#pragma once

#include <vector>
#include <utility>
#include <memory>

#include "common.h"
#include "Chunk.h"
#include "Tile.h"
#include "WorldGen.h"
#include "Entity.h"

namespace Swan {

class World;

class WorldPlane {
public:
	using ID = uint16_t;

	WorldPlane(ID id, World *world, std::shared_ptr<WorldGen> gen):
			id_(id), world_(world), gen_(gen) {}

	Entity &spawnEntity(const std::string &name, const Vec2 &pos);

	bool hasChunk(ChunkPos pos);
	Chunk &getChunk(ChunkPos pos);
	void setTileID(TilePos pos, Tile::ID id);
	Tile &getTile(TilePos pos);

	Entity &spawnPlayer();

	void draw(Win &win);
	void update(Game &game, float dt);
	void tick();

	void debugBox(TilePos pos);

	ID id_;
	World *world_;
	std::shared_ptr<WorldGen> gen_;

private:
	std::map<std::pair<int, int>, std::unique_ptr<Chunk>> chunks_;
	std::vector<std::unique_ptr<Entity>> entities_;
	std::vector<TilePos> debug_boxes_;
};

}
