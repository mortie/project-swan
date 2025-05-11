#pragma once

#include <span>
#include <vector>
#include <deque>
#include <utility>
#include <memory>
#include <unordered_map>
#include <functional>

#include "Clock.h"
#include "common.h"
#include "systems/EntitySystem.h"
#include "systems/FluidSystem.h"
#include "systems/LightSystem.h"
#include "systems/TileSystem.h"
#include "util.h"
#include "Chunk.h"
#include "Tile.h"
#include "WorldGen.h"

namespace Swan {

class World;
class Game;

class WorldPlane final: NonCopyable {
public:
	using ID = uint16_t;

	enum class TickProgress {
		IDLE, FLUID_ONGOING,
	};

	WorldPlane(
		ID id, World *world, std::unique_ptr<WorldGen> gen,
		std::vector<std::unique_ptr<EntityCollection>> &&colls);

	Context getContext();

	EntitySystem &entities() { return entitySystem_; }
	FluidSystem &fluids() { return fluidSystem_; }
	LightSystem &lights() { return lightSystem_; }
	TileSystem &tiles() { return tileSystem_; }

	bool hasChunk(ChunkPos pos);
	Chunk &getChunk(ChunkPos pos);
	Chunk &slowGetChunk(ChunkPos pos);

	EntityRef spawnPlayer();

	bool breakTile(TilePos pos)
	{
		return tileSystem_.breakTile(pos);
	}

	bool placeTile(TilePos pos, Tile::ID id)
	{
		return tileSystem_.placeTile(pos, id);
	}

	void nextTick(std::function<void(const Context &)> cb)
	{
		nextTickA_.push_back(std::move(cb));
	}

	Cygnet::Color backgroundColor();
	void draw(Cygnet::Renderer &rnd);
	void update(float dt);
	bool tick(float dt, RTDeadline deadline);

	ID id_;
	World *world_;
	std::unique_ptr<WorldGen> worldGen_;

private:
	void serialize(proto::WorldPlane::Builder w);
	void deserialize(proto::WorldPlane::Reader r, std::span<Tile::ID> tileMap);

	std::unordered_map<ChunkPos, Chunk> chunks_;
	std::vector<Chunk *> activeChunks_;
	std::vector<std::pair<ChunkPos, Chunk *>> tickChunks_;

	std::deque<Chunk *> chunkInitList_;

	// Callbacks to run on next tick
	std::vector<std::function<void(const Context &)>> nextTickA_;
	std::vector<std::function<void(const Context &)>> nextTickB_;

	TickProgress tickProgress_ = TickProgress::IDLE;
	FluidSystem fluidSystem_{*this};
	EntitySystem entitySystem_;
	LightSystem lightSystem_{*this};
	TileSystem tileSystem_{*this};

	friend Chunk;
	friend World;
};

}
