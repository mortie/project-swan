#include "WGDefault.h"

void WGDefault::genChunk(Swan::WorldPlane &plane, Swan::Chunk &chunk) {
	int height = perlin_.octaveNoise0_1(chunk.pos_.x_ / (64.0 / 16), 8) * 10;
	int depth = perlin_.octaveNoise0_1(chunk.pos_.x_ / (64.0 / 9), 5) * 10 + 10;

	for (int cx = 0; cx < Swan::CHUNK_WIDTH; ++cx) {
		for (int cy = 0; cy < Swan::CHUNK_HEIGHT; ++cy) {
			Swan::TilePos tpos = Swan::TilePos(cx, cy) + Swan::TilePos(
					chunk.pos_.x_ * Swan::CHUNK_WIDTH, chunk.pos_.y_ * Swan::CHUNK_HEIGHT);

			if (tpos.y_ == height)
				chunk.setTileID(Swan::TilePos(cx, cy), tGrass_);
			else if (tpos.y_ > height && tpos.y_ <= depth)
				chunk.setTileID(Swan::TilePos(cx, cy), tDirt_);
			else if (tpos.y_ > depth)
				chunk.setTileID(Swan::TilePos(cx, cy), tStone_);
			else
				chunk.setTileID(Swan::TilePos(cx, cy), tAir_);
		}
	}
}

Swan::Entity &WGDefault::spawnPlayer(Swan::WorldPlane &plane) {
	return plane.spawnEntity("core::player", Swan::Vec2(0, 0));
}
