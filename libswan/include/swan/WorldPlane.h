#pragma once

#include <span>
#include <vector>
#include <deque>
#include <utility>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <mutex>
#include <functional>

#include "common.h"
#include "systems/EntitySystem.h"
#include "systems/FluidSystem.h"
#include "systems/LightSystem.h"
#include "util.h"
#include "Chunk.h"
#include "Tile.h"
#include "WorldGen.h"
#include "FastHashSet.h"

namespace Swan {

class World;
class Game;

struct Raycast {
	bool hit;
	Tile &tile;
	TilePos pos;
	Vec2i face;
};

class WorldPlane final: NonCopyable {
public:
	using ID = uint16_t;

	WorldPlane(
		ID id, World *world, std::unique_ptr<WorldGen> gen,
		std::vector<std::unique_ptr<EntityCollection>> &&colls);

	Context getContext();

	EntitySystem &entities() { return entitySystem_; }
	FluidSystem &fluids() { return fluidSystem_; }
	LightSystem &lights() { return lightSystem_; }

	bool hasChunk(ChunkPos pos);
	Chunk &getChunk(ChunkPos pos);
	Chunk &slowGetChunk(ChunkPos pos);
	void setTileID(TilePos pos, Tile::ID id);
	void setTile(TilePos pos, const std::string &name);
	bool setTileIDWithoutUpdate(TilePos pos, Tile::ID id);

	Tile::ID getTileID(TilePos pos);
	Tile &getTile(TilePos pos);

	EntityRef spawnPlayer();
	bool breakTile(TilePos pos);
	bool placeTile(TilePos pos, Tile::ID);

	void nextTick(std::function<void(const Context &)> cb)
	{
		nextTick_.push_back(std::move(cb));
	}

	Raycast raycast(Vec2 pos, Vec2 direction, float distance);

	Cygnet::Color backgroundColor();
	void draw(Cygnet::Renderer &rnd);
	void update(float dt);
	void tick(float dt);

	void setFluid(TilePos pos, Fluid::ID fluid)
	{
		auto chunkPos = tilePosToChunkPos(pos);
		auto &chunk = getChunk(chunkPos);
		chunk.setFluidID(tilePosToChunkRelPos(pos), fluid);
	}

	void scheduleTileUpdate(TilePos pos)
	{
		scheduledTileUpdates_.push_back(pos);
	}

	ID id_;
	World *world_;
	std::unique_ptr<WorldGen> worldGen_;

private:
	void serialize(sbon::Writer w);
	void deserialize(sbon::Reader r, std::span<Tile::ID> tileMap);

	std::unordered_map<ChunkPos, Chunk> chunks_;
	std::vector<Chunk *> activeChunks_;
	std::vector<std::pair<ChunkPos, Chunk *>> tickChunks_;

	std::deque<Chunk *> chunkInitList_;

	// Tiles to update the next tick
	std::vector<TilePos> scheduledTileUpdates_;
	std::vector<TilePos> scheduledTileUpdatesB_;

	// Callbacks to run on next tick
	std::vector<std::function<void(const Context &)>> nextTick_;
	std::vector<std::function<void(const Context &)>> nextTickB_;

	FluidSystem fluidSystem_{*this};
	EntitySystem entitySystem_;
	LightSystem lightSystem_{*this};

	friend Chunk;
	friend World;
};

}
