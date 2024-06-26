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
#include "traits/BodyTrait.h"
#include "util.h"
#include "Chunk.h"
#include "Tile.h"
#include "WorldGen.h"
#include "Entity.h"
#include "EntityCollection.h"
#include "LightServer.h"
#include "Automata.h"

namespace Swan {

class World;
class Game;

class WorldPlane final: NonCopyable, public LightCallback {
public:
	using ID = uint16_t;

	struct FoundEntity {
		EntityRef ref;
		BodyTrait::Body &body;
	};

	WorldPlane(
		ID id, World *world, std::unique_ptr<WorldGen> gen,
		std::vector<std::unique_ptr<EntityCollection>> &&colls);

	Context getContext();

	EntityRef spawnEntity(const std::string &name, sbon::ObjectReader r);

	template<typename Ent, typename ... Args>
	EntityRef spawnEntity(Args && ... args);

	void despawnEntity(EntityRef ref);

	std::vector<FoundEntity> &getCollidingEntities(BodyTrait::Body &body);
	std::vector<FoundEntity> &getEntitiesInTile(TilePos pos);
	std::vector<FoundEntity> &getEntitiesInArea(Vec2 pos, Vec2 size);
	EntityRef getTileEntity(TilePos pos);

	EntityRef currentEntity();

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

	void setWater(TilePos pos)
	{
		automata_.fillWater(pos);
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
	template<typename Ent>
	EntityCollection &getCollectionOf();
	EntityCollection &getCollectionOf(std::string name);
	EntityCollection &getCollectionOf(std::type_index type);

	NewLightChunk computeLightChunk(const Chunk &chunk);

	std::unordered_map<ChunkPos, Chunk> chunks_;
	std::vector<Chunk *> activeChunks_;
	std::vector<std::pair<ChunkPos, Chunk *>> tickChunks_;
	std::vector<std::unique_ptr<EntityCollection>> entColls_;
	std::unordered_map<std::type_index, EntityCollection *> entCollsByType_;
	std::unordered_map<std::string, EntityCollection *> entCollsByName_;
	EntityCollection *currentEntCol_;

	std::vector<FoundEntity> foundEntitiesRet_;

	std::vector<EntityRef> entDespawnList_;
	std::vector<EntityRef> entDespawnListB_;

	std::deque<Chunk *> chunkInitList_;

	// Tiles to update the next tick
	std::vector<TilePos> scheduledTileUpdates_;
	std::vector<TilePos> scheduledTileUpdatesB_;

	Automata automata_;

	// The lighting server must destruct first. Until it has been destructed,
	// it might call onLightChunkUpdated. If that happens after some other
	// members have destructed, we have a problem.
	// TODO: Rewrite this to not use a callback-based interface.
	std::unique_ptr<LightServer> lighting_;

	// Callbacks to run on next tick
	std::vector<std::function<void(const Context &)>> nextTick_;
	std::vector<std::function<void(const Context &)>> nextTickB_;

	std::unordered_map<TilePos, EntityRef> tileEntities_;

	void despawnTileEntity(TilePos pos);
	void spawnTileEntity(TilePos pos, const std::string &name);

	friend EntityRef;
};

/*
 * WorldPlane
 */

template<typename Ent, typename ... Args>
inline EntityRef WorldPlane::spawnEntity(Args &&... args)
{
	auto ent = getCollectionOf(typeid(Ent)).spawn<Ent, Args...>(
		getContext(), std::forward<Args>(args)...);
	ent->onSpawn(getContext());
	return ent;
}

inline EntityRef WorldPlane::getTileEntity(TilePos pos)
{
	auto it = tileEntities_.find(pos);
	if (it == tileEntities_.end()) {
		return {};
	} else {
		return it->second;
	}
}

inline EntityRef WorldPlane::currentEntity()
{
	return currentEntCol_->currentEntity();
}

inline void WorldPlane::despawnEntity(EntityRef ref)
{
	entDespawnList_.push_back(ref);
}

template<typename Ent>
inline EntityCollection &WorldPlane::getCollectionOf()
{
	return *entCollsByType_.at(typeid(Ent));
}

inline EntityCollection &WorldPlane::getCollectionOf(std::string name)
{
	return *entCollsByName_.at(name);
}

inline EntityCollection &WorldPlane::getCollectionOf(std::type_index type)
{
	return *entCollsByType_.at(type);
}

inline void WorldPlane::nextTick(std::function<void(const Context &)> cb)
{
	nextTick_.push_back(std::move(cb));
}

}
