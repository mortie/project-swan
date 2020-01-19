#include "WGDefault.h"

static int grassLevel(const siv::PerlinNoise &perlin, int x) {
	return (int)(perlin.noise(x / 50.0, 0) * 13);
}

static int stoneLevel(const siv::PerlinNoise &perlin, int x) {
	return (int)(perlin.noise(x / 50.0, 10) * 10) + 10;
}

Swan::Tile::ID WGDefault::genTile(Swan::TilePos pos) {
	int grass_level = grassLevel(perlin_, pos.x);
	int stone_level = stoneLevel(perlin_, pos.x);

	if (pos.y > stone_level)
		return tStone_;
	else if (pos.y > grass_level)
		return tDirt_;
	else if (pos.y == grass_level)
		return tGrass_;
	else
		return tAir_;
}

void WGDefault::genChunk(Swan::WorldPlane &plane, Swan::Chunk &chunk) {
	for (int cx = 0; cx < Swan::CHUNK_WIDTH; ++cx) {
		int tilex = chunk.pos_.x * Swan::CHUNK_WIDTH + cx;

		for (int cy = 0; cy < Swan::CHUNK_HEIGHT; ++cy) {
			int tiley = chunk.pos_.y * Swan::CHUNK_HEIGHT + cy;

			Swan::TilePos pos(tilex, tiley);
			Swan::Chunk::RelPos rel(cx, cy);
			chunk.setTileID(rel, genTile(pos));
		}
	}
}

Swan::BodyTrait::HasBody *WGDefault::spawnPlayer(Swan::WorldPlane &plane) {
	int x = 0;
	return dynamic_cast<Swan::BodyTrait::HasBody *>(
		plane.spawnEntity("core::player", Swan::SRFFloatArray{
			(float)x, (float)grassLevel(perlin_, x) - 4 }));
}
