#include "DefaultWorldGen.h"

#include <algorithm>

#include "entities/PlayerEntity.h"

static int getGrassLevel(const siv::PerlinNoise &perlin, int x) {
	return (int)(perlin.noise(x / 50.0, 0) * 13);
}

static int getStoneLevel(const siv::PerlinNoise &perlin, int x) {
	return (int)(perlin.noise(x / 50.0, 10) * 10) + 10;
}

void DefaultWorldGen::drawBackground(
		const Swan::Context &ctx, Cygnet::Renderer &rnd, Swan::Vec2 pos) {
	int texmin = 10;
	//int texmax = 20;

	if (pos.y > texmin) {
		/*
		SDL_Texture *tex = bgCave_.texture_.get();

		Uint8 alpha =  std::clamp(
			(pos.y - texmin) / (texmax - texmin), 0.0f, 1.0f) * 255;
		Swan::TexAlphaMod amod(tex, alpha);


		Swan::Draw::parallaxBackground(
			win, tex, std::nullopt, std::nullopt,
			pos.x * Swan::TILE_SIZE, pos.y * Swan::TILE_SIZE, 0.7);
		TODO */
	}
}

SDL_Color DefaultWorldGen::backgroundColor(Swan::Vec2 pos) {
	float y = pos.y;
	return Swan::Draw::linearGradient(y, {
		{    0, { 128, 220, 250, 255 } },
		{   70, { 107,  87,   5, 255 } },
		{  100, { 107,  87,   5, 255 } },
		{  200, {  20,  20,  23, 255 } },
		{  300, {  20,  20,  23, 255 } },
		{  500, {  25,  10,  10, 255 } },
		{ 1000, {  65,  10,  10, 255 } } });
}

Swan::Tile::ID DefaultWorldGen::genTile(Swan::TilePos pos) {
	int grassLevel = getGrassLevel(perlin_, pos.x);
	int stoneLevel = getStoneLevel(perlin_, pos.x);

	// Caves
	if (pos.y > grassLevel + 7 && perlin_.noise(pos.x / 43.37, pos.y / 16.37) > 0.2)
		return tAir_;

	if (pos.y > stoneLevel)
		return tStone_;
	else if (pos.y > grassLevel)
		return tDirt_;
	else if (pos.y == grassLevel)
		return tGrass_;
	else
		return tAir_;
}

void DefaultWorldGen::genChunk(Swan::WorldPlane &plane, Swan::Chunk &chunk) {
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

Swan::EntityRef DefaultWorldGen::spawnPlayer(const Swan::Context &ctx) {
	int x = 0;
	return ctx.plane.spawnEntity<PlayerEntity>(
		ctx, Swan::Vec2{ (float)x, (float)getGrassLevel(perlin_, x) - 4 });
}
