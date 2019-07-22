#include "WorldPlane.h"

#include "World.h"

namespace Swan {

Chunk &WorldPlane::getChunk(int x, int y) {
	auto coord = Coord(x / CHUNK_WIDTH, y / CHUNK_HEIGHT);
	auto it = chunks_.find(coord);

	if (it == chunks_.end()) {
		it = chunks_.emplace(coord, Chunk(coord.first, coord.second)).first;
		gen_->genChunk(it->second, coord.first, coord.second);
		it->second.redraw(world_->tile_map_);
	}

	return it->second;
}

void WorldPlane::setTileID(int x, int y, Tile::ID id) {
	getChunk(x, y).setTileID(world_->tile_map_, x % CHUNK_WIDTH, y % CHUNK_HEIGHT, id);
}

Tile &WorldPlane::getTile(int x, int y) {
	return getChunk(x, y).getTile(world_->tile_map_, x % CHUNK_WIDTH, y % CHUNK_HEIGHT);
}

void WorldPlane::draw(Win &win) {
	for (auto &p: chunks_) {
		p.second.draw(win);
	}
}

void WorldPlane::update(float dt) {
}

void WorldPlane::tick() {
}

}
