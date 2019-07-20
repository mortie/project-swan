#include "WorldPlane.h"

#include "World.h"

namespace Swan {

void WorldPlane::setTile(int x, int y, Tile::ID id) {
	int chx = x / CHUNK_WIDTH;
	int chy = y / CHUNK_HEIGHT;
	int rx = x % CHUNK_WIDTH;
	int ry = y % CHUNK_HEIGHT;

	Chunk *chunk = NULL;
	for (auto &ch: chunks_) {
		if (ch.x_ == chx && ch.y_ == chy) {
			chunk = &ch;
			break;
		}
	}

	if (chunk == NULL) {
		chunks_.push_back(Chunk(chx, chy));
		chunk = &chunks_.back();
		chunk->fill(world_->tile_map_, 0);
	}

	chunk->setTile(world_->tile_map_, rx, ry, id);
}

void WorldPlane::draw(Win &win) {
	for (auto &chunk: chunks_) {
		chunk.draw(win);
	}
}

void WorldPlane::update(float dt) {
}

void WorldPlane::tick() {
}

}
