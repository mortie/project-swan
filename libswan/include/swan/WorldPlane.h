#pragma once

#include <vector>
#include <deque>
#include <utility>
#include <memory>
#include <map>
#include <set>

#include "common.h"
#include "traits/BodyTrait.h"
#include "util.h"
#include "Chunk.h"
#include "Tile.h"
#include "WorldGen.h"
#include "Entity.h"

namespace Swan {

class World;
class Game;

class WorldPlane: NonCopyable {
public:
	using ID = uint16_t;

	WorldPlane(ID id, World *world, std::unique_ptr<WorldGen> gen):
			id_(id), world_(world), gen_(std::move(gen)) {}

	Entity *spawnEntity(const std::string &name, const Entity::PackObject &params);
	Entity *spawnEntity(std::unique_ptr<Entity> ent);
	void despawnEntity(Entity &ent);

	Context getContext();

	bool hasChunk(ChunkPos pos);
	Chunk &getChunk(ChunkPos pos);
	void setTileID(TilePos pos, Tile::ID id);
	void setTile(TilePos pos, const std::string &name);
	Tile::ID getTileID(TilePos pos);
	Tile &getTile(TilePos pos);

	Iter<Entity *> getEntsInArea(Vec2 center, float radius);

	template<typename T>
	Iter<T *>getEntsOfType() {
		return mapFilter(entities_.begin(), entities_.end(), [](std::unique_ptr<Entity> &ent) -> std::optional<T *> {
			if (T *e = dynamic_cast<T *>(ent.get()); e != nullptr)
				return e;
			return std::nullopt;
		});
	}

	BodyTrait::HasBody *spawnPlayer();
	void breakBlock(TilePos pos);

	SDL_Color backgroundColor();
	void draw(Win &win);
	void update(float dt);
	void tick(float dt);

	void debugBox(TilePos pos);

	ID id_;
	World *world_;
	std::unique_ptr<WorldGen> gen_;

private:
	std::map<std::pair<int, int>, Chunk> chunks_;
	std::vector<Chunk *> active_chunks_;
	std::vector<std::unique_ptr<Entity>> entities_;

	std::deque<Chunk *> chunk_init_list_;
	std::vector<std::unique_ptr<Entity>> spawn_list_;
	std::vector<Entity *> despawn_list_;
	std::vector<TilePos> debug_boxes_;
};

}
