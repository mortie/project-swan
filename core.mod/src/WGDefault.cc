#include "WGDefault.h"

void WGDefault::genChunk(Swan::WorldPlane &plane, Swan::Chunk &chunk) {
	for (int cx = 0; cx < Swan::CHUNK_WIDTH; ++cx) {
		for (int cy = 0; cy < Swan::CHUNK_HEIGHT; ++cy) {
			Swan::TilePos tpos = Swan::TilePos(cx, cy) + chunk.pos_ * Swan::TILE_SIZE;
			if (tpos.y_ == 3)
				chunk.tiles_[cx][cy] = tGrass_;
			else if (tpos.y_ > 3 && tpos.y_ <= 5)
				chunk.tiles_[cx][cy] = tDirt_;
			else if (tpos.y_ > 5)
				chunk.tiles_[cx][cy] = tStone_;
			else
				chunk.tiles_[cx][cy] = tAir_;
		}
	}
}

Swan::Entity &WGDefault::spawnPlayer(Swan::WorldPlane &plane) {
	return plane.spawnEntity("core::player", Swan::Vec2(0, 0));
}
