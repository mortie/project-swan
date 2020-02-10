#include "WGDefault.h"

#include <algorithm>

static int grassLevel(const siv::PerlinNoise &perlin, int x) {
	return (int)(perlin.noise(x / 50.0, 0) * 13);
}

static int stoneLevel(const siv::PerlinNoise &perlin, int x) {
	return (int)(perlin.noise(x / 50.0, 10) * 10) + 10;
}

void WGDefault::drawBackground(const Swan::Context &ctx, Swan::Win &win, Swan::Vec2 pos) {
	int texmin = 10;
	int texmax = 20;

	if (pos.y > texmin) {
		SDL_Texture *tex = bgCave_.texture_.get();

		Uint8 alpha =  std::clamp(
			(pos.y - texmin) / (texmax - texmin), 0.0f, 1.0f) * 255;
		Swan::TexAlphaMod amod(tex, alpha);


		Swan::Draw::parallaxBackground(
			win, tex, std::nullopt, std::nullopt,
			pos.x * Swan::TILE_SIZE, pos.y * Swan::TILE_SIZE, 0.7);
	}
}

SDL_Color WGDefault::backgroundColor(Swan::Vec2 pos) {
	float y = pos.y;
	float deep = 20;
	float deeper = deep + 100;
	if (y < deep) {
		return Swan::Draw::linearColor(
			{ 128, 220, 250, 255 },
			{ 107, 87, 5, 255 },
			y / deep);
	} else if (y < deeper) {
		return Swan::Draw::linearColor(
			{ 107, 87, 5, 255 },
			{ 15, 3, 3, 255 },
			(y - deep) / (deeper - deep));
	} else {
		return { 15, 3, 3, 255 };
	}
}

Swan::Tile::ID WGDefault::genTile(Swan::TilePos pos) {
	int grass_level = grassLevel(perlin_, pos.x);
	int stone_level = stoneLevel(perlin_, pos.x);

	// Caves
	if (pos.y > grass_level + 7 && perlin_.noise(pos.x / 43.37, pos.y / 16.37) > 0.2)
		return tAir_;

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
			chunk.setTileData(rel, genTile(pos));
		}
	}
}

Swan::BodyTrait::HasBody *WGDefault::spawnPlayer(Swan::WorldPlane &plane) {
	int x = 0;
	return dynamic_cast<Swan::BodyTrait::HasBody *>(
		plane.spawnEntity("core::player", Swan::SRFFloatArray{
			(float)x, (float)grassLevel(perlin_, x) - 4 }));
}
