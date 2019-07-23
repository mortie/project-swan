#include "WorldPlane.h"

#include "World.h"

namespace Swan {

static Chunk::ChunkPos chunkPos(int x, int y) {
	int chx = x / CHUNK_WIDTH;
	if (x < 0 && x % CHUNK_WIDTH != 0) chx -= 1;
	int chy = y / CHUNK_HEIGHT;
	if (y < 0 && y % CHUNK_HEIGHT != 0) chy -= 1;
	return Chunk::ChunkPos(chx, chy);
}

static Chunk::RelPos relPos(int x, int y) {
	int rx = x % CHUNK_WIDTH;
	if (rx < 0) rx += CHUNK_WIDTH;
	int ry = y % CHUNK_HEIGHT;
	if (ry < 0) ry += CHUNK_HEIGHT;
	return Chunk::RelPos(rx, ry);
}

Chunk &WorldPlane::getChunk(int x, int y) {
	Chunk::ChunkPos pos = chunkPos(x, y);
	auto it = chunks_.find(pos);

	if (it == chunks_.end()) {
		it = chunks_.emplace(pos, Chunk(pos)).first;
		gen_->genChunk(it->second, pos.x_, pos.y_);
		it->second.redraw(world_->tile_map_);
	}

	return it->second;
}

void WorldPlane::setTileID(int x, int y, Tile::ID id) {
	getChunk(x, y).setTileID(world_->tile_map_, relPos(x, y), id);
}

Tile &WorldPlane::getTile(int x, int y) {
	return getChunk(x, y).getTile(world_->tile_map_, relPos(x, y));
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
