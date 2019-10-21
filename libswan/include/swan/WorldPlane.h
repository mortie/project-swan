#pragma once

#include <vector>
#include <utility>
#include <memory>
#include <map>
#include <set>

#include "common.h"
#include "Chunk.h"
#include "Tile.h"
#include "WorldGen.h"
#include "Entity.h"

namespace Swan {

class World;
class Game;

class WorldPlane {
public:
	using ID = uint16_t;

	WorldPlane(ID id, World *world, std::shared_ptr<WorldGen> gen):
			id_(id), world_(world), gen_(std::move(gen)) {}

	Entity &spawnEntity(const std::string &name, const SRF &params);
	void despawnEntity(Entity &ent);

	Context getContext();

	bool hasChunk(ChunkPos pos);
	Chunk &getChunk(ChunkPos pos);
	void setTileID(TilePos pos, Tile::ID id);
	void setTile(TilePos pos, const std::string &name);
	Tile::ID getTileID(TilePos pos);
	Tile &getTile(TilePos pos);

	Entity &spawnPlayer();
	void breakBlock(TilePos pos);

	void draw(Win &win);
	void update(float dt);
	void tick(float dt);

	void debugBox(TilePos pos);

	ID id_;
	World *world_;
	std::shared_ptr<WorldGen> gen_;

private:
	std::map<std::pair<int, int>, Chunk> chunks_;
	std::set<Chunk *> active_chunks_;
	std::vector<std::unique_ptr<Entity>> entities_;
	std::vector<std::unique_ptr<Entity>> spawn_list_;
	std::vector<Entity *> despawn_list_;
	std::vector<TilePos> debug_boxes_;
};

}
