#include "WorldPlane.h"

namespace Swan {

void WorldPlane::setTile(int x, int y, Tile::TileID tile) {
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
		chunks_.push_back(Chunk());
		chunk = &chunks_.back();
		chunk->clear();
	}

	chunk->setTile(rx, ry, tile);
}

void WorldPlane::draw(Win &win) {
}

void WorldPlane::update(float dt) {
}

void WorldPlane::tick() {
}

}
