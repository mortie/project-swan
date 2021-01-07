#pragma once

#include <vector>
#include <deque>
#include <utility>
#include <memory>
#include <map>
#include <set>
#include <typeindex>
#include <mutex>

#include "common.h"
#include "traits/BodyTrait.h"
#include "util.h"
#include "Chunk.h"
#include "Tile.h"
#include "WorldGen.h"
#include "Entity.h"
#include "Collection.h"
#include "LightServer.h"

namespace Swan {

class World;
class Game;

class WorldPlane final: NonCopyable, public LightCallback {
public:
	using ID = uint16_t;

	WorldPlane(
			ID id, World *world, std::unique_ptr<WorldGen> gen,
			std::vector<std::unique_ptr<EntityCollection>> &&colls);

	EntityRef spawnEntity(const std::string &name, const Entity::PackObject &params);
	template<typename Ent, typename... Args>
	EntityRef spawnEntity(Args&&... args);

	Context getContext();

	bool hasChunk(ChunkPos pos);
	Chunk &getChunk(ChunkPos pos);
	Chunk &slowGetChunk(ChunkPos pos);
	void setTileID(TilePos pos, Tile::ID id);
	void setTile(TilePos pos, const std::string &name);

	template<typename Ent>
	EntityCollection &getCollectionOf();
	EntityCollection &getCollectionOf(std::string name);
	EntityCollection &getCollectionOf(std::type_index type);

	Tile::ID getTileID(TilePos pos);
	Tile &getTile(TilePos pos);

	Iter<Entity *> getEntsInArea(Vec2 center, float radius);

	template<typename T>
	Iter<T *>getEntsOfType() {
		return Iter<T *>([] { return std::nullopt; });
		/* TODO
		return mapFilter(entities_.begin(), entities_.end(), [](std::unique_ptr<Entity> &ent) -> std::optional<T *> {
			if (T *e = dynamic_cast<T *>(ent.get()); e != nullptr)
				return e;
			return std::nullopt;
		});
		*/
	}

	EntityRef spawnPlayer();
	void breakTile(TilePos pos);

	SDL_Color backgroundColor();
	void draw(Win &win);
	void update(float dt);
	void tick(float dt);

	void debugBox(TilePos pos);

	void addLight(TilePos pos, float level);
	void removeLight(TilePos pos, float level);

	// LightingCallback implementation
	void onLightChunkUpdated(const LightChunk &chunk, Vec2i pos) final;

	ID id_;
	World *world_;
	std::unique_ptr<WorldGen> gen_;
	std::mutex mut_;

private:
	std::unique_ptr<LightServer> lighting_;

	std::map<std::pair<int, int>, Chunk> chunks_;
	std::vector<Chunk *> activeChunks_;
	std::vector<std::pair<ChunkPos, Chunk *>> tickChunks_;
	std::vector<std::unique_ptr<EntityCollection>> entColls_;
	std::unordered_map<std::type_index, EntityCollection *> entCollsByType_;
	std::unordered_map<std::string, EntityCollection *> entCollsByName_;

	std::deque<Chunk *> chunkInitList_;
	std::vector<TilePos> debugBoxes_;
};

/*
 * WorldPlane
 */

template<typename Ent, typename... Args>
inline EntityRef WorldPlane::spawnEntity(Args&&... args) {
	return getCollectionOf(typeid(Ent)).spawn<Ent, Args...>(std::forward<Args>(args)...);
}

template<typename Ent>
inline EntityCollection &WorldPlane::getCollectionOf() {
	return *entCollsByType_.at(typeid(Ent));
}

inline EntityCollection &WorldPlane::getCollectionOf(std::string name) {
	return *entCollsByName_.at(name);
}

inline EntityCollection &WorldPlane::getCollectionOf(std::type_index type) {
	return *entCollsByType_.at(type);
}

}
