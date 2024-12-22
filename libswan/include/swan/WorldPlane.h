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
#include "util.h"
#include "Chunk.h"
#include "Tile.h"
#include "WorldGen.h"
#include "LightServer.h"
#include "FastHashSet.h"

namespace Swan {

class World;
class Game;

class WorldPlane final: NonCopyable, public LightCallback {
public:
	using ID = uint16_t;

	WorldPlane(
		ID id, World *world, std::unique_ptr<WorldGen> gen,
		std::vector<std::unique_ptr<EntityCollection>> &&colls);

	Context getContext();

	EntitySystem &entities() { return entitySystem_; }
	FluidSystem &fluids() { return fluidSystem_; }

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

	void nextTick(std::function<void(const Context &)> cb);

	struct Raycast {
		bool hit;
		Tile &tile;
		TilePos pos;
		Vec2i face;
	};

	Raycast raycast(Vec2 pos, Vec2 direction, float distance);

	Cygnet::Color backgroundColor();
	void draw(Cygnet::Renderer &rnd);
	void update(float dt);
	void tick(float dt);

	void addLight(TilePos pos, float level);
	void removeLight(TilePos pos, float level);

	void setFluid(TilePos pos, Fluid::ID fluid)
	{
		auto chunkPos = tilePosToChunkPos(pos);
		auto &chunk = getChunk(chunkPos);
		chunk.setFluidID(tilePosToChunkRelPos(pos), fluid);
	}

	// LightingCallback implementation
	void onLightChunkUpdated(const LightChunk &chunk, Vec2i pos) final;

	void serialize(sbon::Writer w);
	void deserialize(sbon::Reader r, std::span<Tile::ID> tileMap);

	void scheduleTileUpdate(TilePos pos)
	{
		scheduledTileUpdates_.push_back(pos);
	}

	ID id_;
	World *world_;
	std::unique_ptr<WorldGen> worldGen_;
	std::mutex mut_;

private:
	NewLightChunk computeLightChunk(const Chunk &chunk);

	std::unordered_map<ChunkPos, Chunk> chunks_;
	std::vector<Chunk *> activeChunks_;
	std::vector<std::pair<ChunkPos, Chunk *>> tickChunks_;

	std::deque<Chunk *> chunkInitList_;

	// Tiles to update the next tick
	std::vector<TilePos> scheduledTileUpdates_;
	std::vector<TilePos> scheduledTileUpdatesB_;

	// The lighting server must destruct first. Until it has been destructed,
	// it might call onLightChunkUpdated. If that happens after some other
	// members have destructed, we have a problem.
	// TODO: Rewrite this to not use a callback-based interface.
	std::unique_ptr<LightServer> lighting_;

	// Callbacks to run on next tick
	std::vector<std::function<void(const Context &)>> nextTick_;
	std::vector<std::function<void(const Context &)>> nextTickB_;

	FluidSystem fluidSystem_{*this};
	EntitySystem entitySystem_;

	friend EntityRef;
	friend Chunk;
};

inline void WorldPlane::nextTick(std::function<void(const Context &)> cb)
{
	nextTick_.push_back(std::move(cb));
}

}
