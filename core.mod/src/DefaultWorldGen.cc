#include "DefaultWorldGen.h"

#include "entities/PlayerEntity.h"
#include "entities/SpiderEntity.h"
#include "swan-common/constants.h"
#include "world/util.h"
#include "worldgen/StructureDef.h"

namespace CoreMod {

static constexpr int GEN_PADDING = 16;

static int getGrassLevel(const siv::PerlinNoise &perlin, int x)
{
	return (int)(perlin.noise2D(x / 50.0, 0) * 13);
}

static int getStoneLevel(const siv::PerlinNoise &perlin, int x)
{
	return (int)(perlin.noise2D(x / 50.0, 10) * 10) + 10;
}

static int getPlayerX(const siv::PerlinNoise &perlin)
{
	return 0;
}

void DefaultWorldGen::drawBackground(
	const Swan::Context &ctx, Cygnet::Renderer &rnd, Swan::Vec2 pos)
{
	// TODO: Do something interesting?
}

Cygnet::Color DefaultWorldGen::backgroundColor(Swan::Vec2 pos)
{
	float y = pos.y;

	return Swan::Draw::linearGradient(y, {
		{0, Cygnet::ByteColor{128, 220, 250}},
		{70, Cygnet::ByteColor{107, 87, 5}},
		{100, Cygnet::ByteColor{107, 87, 5}},
		{200, Cygnet::ByteColor{20, 20, 23}},
		{300, Cygnet::ByteColor{20, 20, 23}},
		{500, Cygnet::ByteColor{25, 10, 10}},
		{1000, Cygnet::ByteColor{65, 10, 10}},
	});
}

Swan::Tile::ID DefaultWorldGen::genTile(
	Swan::TilePos pos,
	int grassLevel,
	int stoneLevel)
{
	// Caves
	if (pos.y > grassLevel + 7 && perlin_.noise2D(pos.x / 43.37, pos.y / 16.37) > 0.2) {
		return tAir_;
	}

	if (pos.y > stoneLevel) {
		return tStone_;
	}
	else if (pos.y > grassLevel) {
		return tDirt_;
	}
	else if (pos.y == grassLevel) {
		return tGrass_;
	}
	else if (pos.y == grassLevel - 1 && perlin_.noise2D(pos.x / 20.6, 0) > 0.2) {
		return tTallGrass_;
	}
	else{
		return tAir_;
	}
}

void DefaultWorldGen::genChunk(Swan::WorldPlane &plane, Swan::Chunk &chunk)
{
	Swan::TilePos pos = chunk.pos_ * Swan::TilePos{
		Swan::CHUNK_WIDTH, Swan::CHUNK_HEIGHT}; 

	constexpr int GEN_WIDTH = SwanCommon::CHUNK_WIDTH + GEN_PADDING * 2;
	constexpr int GEN_HEIGHT = SwanCommon::CHUNK_HEIGHT + GEN_PADDING * 2;

	int grassLevels[GEN_WIDTH];
	int stoneLevels[GEN_WIDTH];
	for (int rx = 0; rx < GEN_WIDTH; ++rx) {
		int x = (chunk.pos_.x * Swan::CHUNK_WIDTH) - GEN_PADDING + rx;
		grassLevels[rx] = getGrassLevel(perlin_, x);
		stoneLevels[rx] = getStoneLevel(perlin_, x);
	}

	StructureDef::Meta meta = {
		.grassLevels = grassLevels,
		.stoneLevels = stoneLevels,
		.world = *plane.world_,
	};

	for (int cx = 0; cx < Swan::CHUNK_WIDTH; ++cx) {
		int tilex = pos.x + cx;

		for (int cy = 0; cy < Swan::CHUNK_HEIGHT; ++cy) {
			int tiley = pos.y + cy;

			Swan::TilePos pos(tilex, tiley);
			Swan::ChunkRelPos rel(cx, cy);
			chunk.setTileData(rel, genTile(
				pos,
				grassLevels[cx + GEN_PADDING],
				stoneLevels[cx + GEN_PADDING]));
		}
	}

	structureMap_.clear();
	treeDef_.generateArea(
		meta, pos.add(-GEN_PADDING, -GEN_PADDING),
		{GEN_WIDTH, GEN_HEIGHT}, structureMap_);

	for (auto &[tpos, tile]: structureMap_) {
		if (
				tpos.x < pos.x ||
				tpos.y < pos.y ||
				tpos.x > pos.x + Swan::CHUNK_WIDTH ||
				tpos.y > pos.y + Swan::CHUNK_HEIGHT) {
			continue;
		}

		chunk.setTileData(tpos - pos, tile);
	}
}

Swan::EntityRef DefaultWorldGen::spawnPlayer(const Swan::Context &ctx)
{
	int x = getPlayerX(perlin_);

	return ctx.plane.spawnEntity<PlayerEntity>(
		Swan::Vec2{(float)x, (float)getGrassLevel(perlin_, x) - 2});
}

}
