#include "WGDefault.h"

void WGDefault::genChunk(Swan::WorldPlane &plane, Swan::Chunk &chunk) {
	int height = (int)(perlin_.octaveNoise0_1(chunk.pos_.x / (64.0 / 16), 8) * 10);
	int depth = (int)(perlin_.octaveNoise0_1(chunk.pos_.x / (64.0 / 9), 5) * 10 + 10);

	for (int cx = 0; cx < Swan::CHUNK_WIDTH; ++cx) {
		for (int cy = 0; cy < Swan::CHUNK_HEIGHT; ++cy) {
			Swan::TilePos tpos = Swan::TilePos(cx, cy) + Swan::TilePos(
					chunk.pos_.x * Swan::CHUNK_WIDTH, chunk.pos_.y * Swan::CHUNK_HEIGHT);

			if (tpos.y == height)
				chunk.setTileID(Swan::TilePos(cx, cy), tGrass_);
			else if (tpos.y > height && tpos.y <= depth)
				chunk.setTileID(Swan::TilePos(cx, cy), tDirt_);
			else if (tpos.y > depth)
				chunk.setTileID(Swan::TilePos(cx, cy), tStone_);
			else
				chunk.setTileID(Swan::TilePos(cx, cy), tAir_);
		}
	}
}

Swan::Entity &WGDefault::spawnPlayer(Swan::WorldPlane &plane) {
	return plane.spawnEntity("core::player", Swan::SRFFloatArray{0, 0});
}
