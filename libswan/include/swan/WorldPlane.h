#pragma once

#include <vector>
#include <deque>
#include <utility>
#include <memory>
#include <map>
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
		std::vector<std::unique_ptr<EntityCollection> > &&colls);

	Context getContext();

	EntityRef spawnEntity(const std::string &name, const Entity::PackObject &params);

	template<typename Ent, typename ... Args>
	EntityRef spawnEntity(Args && ... args);

	void despawnEntity(EntityRef ref);

	std::vector<FoundEntity> &getCollidingEntities(BodyTrait::Body &body);
	std::vector<FoundEntity> &getEntitiesInTile(TilePos pos);

	EntityRef currentEntity();

	bool hasChunk(ChunkPos pos);
	Chunk &getChunk(ChunkPos pos);
	Chunk &slowGetChunk(ChunkPos pos);
	void setTileID(TilePos pos, Tile::ID id);
	void setTile(TilePos pos, const std::string &name);
	void setTileIDWithoutUpdate(TilePos pos, Tile::ID id);

	Tile::ID getTileID(TilePos pos);
	Tile &getTile(TilePos pos);

	EntityRef spawnPlayer();
	void breakTile(TilePos pos);

	void nextTick(std::function<void(const Context &)> cb);

	Cygnet::Color backgroundColor();
	void draw(Cygnet::Renderer &rnd);
	void ui();
	void update(float dt);
	void tick(float dt);

	void addLight(TilePos pos, float level);
	void removeLight(TilePos pos, float level);

	// LightingCallback implementation
	void onLightChunkUpdated(const LightChunk &chunk, Vec2i pos) final;

	ID id_;
	World *world_;
	std::unique_ptr<WorldGen> gen_;
	std::mutex mut_;

private:
	template<typename Ent>
	EntityCollection &getCollectionOf();
	EntityCollection &getCollectionOf(std::string name);
	EntityCollection &getCollectionOf(std::type_index type);

	std::map<std::pair<int, int>, Chunk> chunks_;
	std::vector<Chunk *> activeChunks_;
	std::vector<std::pair<ChunkPos, Chunk *> > tickChunks_;
	std::vector<std::unique_ptr<EntityCollection> > entColls_;
	std::unordered_map<std::type_index, EntityCollection *> entCollsByType_;
	std::unordered_map<std::string, EntityCollection *> entCollsByName_;
	EntityCollection *currentEntCol_;

	std::vector<FoundEntity> foundEntitiesRet_;

	std::vector<EntityRef> entDespawnList_;
	std::deque<Chunk *> chunkInitList_;

	// Tiles to update the next tick
	std::vector<TilePos> scheduledTileUpdates_;

	// The lighting server must destruct first. Until it has been destructed,
	// it might call onLightChunkUpdated. If that happens after some other
	// members have destructed, we have a problem.
	// TODO: Rewrite this to not use a callback-based interface.
	std::unique_ptr<LightServer> lighting_;

	// Callbacks to run on next tick
	std::vector<std::function<void(const Context &)> > nextTick_;
};

/*
 * WorldPlane
 */

template<typename Ent, typename ... Args>
inline EntityRef WorldPlane::spawnEntity(Args &&... args)
{
	return getCollectionOf(typeid(Ent)).spawn<Ent, Args...>(
		getContext(), std::forward<Args>(args)...);
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
