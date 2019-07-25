#pragma once

#include <vector>
#include <utility>
#include <memory>

#include "common.h"
#include "Chunk.h"
#include "Tile.h"
#include "TileMap.h"
#include "WorldGen.h"
#include "Entity.h"

namespace Swan {

class World;

class WorldPlane {
public:
	using ID = uint16_t;

	WorldPlane(ID id, World *world, std::shared_ptr<WorldGen> gen):
		id_(id), world_(world), gen_(gen) {
			getChunk(0, 0); // Create the initial chunk
		}

	Entity &spawnEntity(const std::string &name, const Vec2 &pos);

	Chunk &getChunk(int x, int y);
	void setTileID(int x, int y, Tile::ID id);
	Tile &getTile(int x, int y);

	void draw(Win &win);
	void update(float dt);
	void tick();

	ID id_;
	World *world_;
	std::shared_ptr<WorldGen> gen_;

private:
	std::map<std::pair<int, int>, std::unique_ptr<Chunk>> chunks_;
	std::vector<std::unique_ptr<Entity>> entities_;
};

}
