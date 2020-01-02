#include "WGDefault.h"

static int getHeightAt(const siv::PerlinNoise &perlin, int tilex) {
	return (int)(perlin.noise(tilex / 50.0, 0) * 13);
}

static int getDepthAt(const siv::PerlinNoise &perlin, int tilex) {
	return (int)(perlin.noise(tilex / 50.0, 10) * 10) + 10;
}

void WGDefault::genChunk(Swan::WorldPlane &plane, Swan::Chunk &chunk) {

	for (int cx = 0; cx < Swan::CHUNK_WIDTH; ++cx) {
		int tilex = chunk.pos_.x * Swan::CHUNK_WIDTH + cx;

		int height = getHeightAt(perlin_, tilex);
		int depth = getDepthAt(perlin_, tilex);
		if (depth <= height) depth = height + 1;

		for (int cy = 0; cy < Swan::CHUNK_HEIGHT; ++cy) {
			int tiley = chunk.pos_.y * Swan::CHUNK_HEIGHT + cy;
			Swan::TilePos tpos = Swan::TilePos(tilex, tiley);

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

Swan::BodyTrait::HasBody *WGDefault::spawnPlayer(Swan::WorldPlane &plane) {
	return dynamic_cast<Swan::BodyTrait::HasBody *>(
		plane.spawnEntity("core::player", Swan::SRFFloatArray{ 0, (float)getHeightAt(perlin_, 0) - 4 }));
}
