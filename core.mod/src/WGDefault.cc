#include "WGDefault.h"

void WGDefault::genChunk(Swan::WorldPlane &plane, Swan::Chunk &chunk) {
	for (int cx = 0; cx < Swan::CHUNK_WIDTH; ++cx) {
		for (int cy = 0; cy < Swan::CHUNK_HEIGHT; ++cy) {
			if (chunk.pos_.y_ == 0 && cy == 3)
				chunk.tiles_[cx][cy] = tGrass_;
			else
				chunk.tiles_[cx][cy] = tAir_;
		}
	}
}

Swan::Entity &WGDefault::spawnPlayer(Swan::WorldPlane &plane) {
	return plane.spawnEntity("core::player", Swan::Vec2(0, 0));
}
