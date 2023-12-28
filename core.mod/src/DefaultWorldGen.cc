#include "DefaultWorldGen.h"

#include "entities/PlayerEntity.h"
#include "entities/SpiderEntity.h"

namespace CoreMod {

static int getGrassLevel(const siv::PerlinNoise &perlin, int x)
{
	return (int)(perlin.noise(x / 50.0, 0) * 13);
}

static int getStoneLevel(const siv::PerlinNoise &perlin, int x)
{
	return (int)(perlin.noise(x / 50.0, 10) * 10) + 10;
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

Swan::Tile::ID DefaultWorldGen::genTile(Swan::TilePos pos)
{
	int grassLevel = getGrassLevel(perlin_, pos.x);
	int stoneLevel = getStoneLevel(perlin_, pos.x);

	// Caves
	if (pos.y > grassLevel + 7 && perlin_.noise(pos.x / 43.37, pos.y / 16.37) > 0.2) {
		return tAir_;
	}

	// Trees
	if (pos.y == grassLevel - 1) {
		constexpr int treeProb = 4;
		bool spawnTree = Swan::random(pos.x) % treeProb == 0;
		if (spawnTree) {
			// Avoid trees which are too close
			for (int rx = 1; rx <= 5; ++rx) {
				if (Swan::random(pos.x + rx) % treeProb == 0) {
					spawnTree = false;
					break;
				}
			}

			if (spawnTree) {
				return tTreeSeeder_;
			}
		}
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
	else if (pos.y == grassLevel - 1 && perlin_.noise(pos.x / 20.6, 0) > 0.2) {
		return tTallGrass_;
	}
	else{
		return tAir_;
	}
}

void DefaultWorldGen::initializeTile(const Swan::Context &ctx, Swan::TilePos pos)
{
	int grassLevel = getGrassLevel(perlin_, pos.x);

	int playerX = getPlayerX(perlin_);
	int playerDist = abs(pos.x - playerX);

	// Spawn mobs
	if (pos.y == grassLevel - 1 && playerDist > 20) {
		double r = perlin_.noise(pos.x * 87.411, 10);
		if (r < -0.75) {
			ctx.plane.spawnEntity<SpiderEntity>(pos);
		}
	}
}

void DefaultWorldGen::genChunk(Swan::WorldPlane &plane, Swan::Chunk &chunk)
{
	for (int cx = 0; cx < Swan::CHUNK_WIDTH; ++cx) {
		int tilex = chunk.pos_.x * Swan::CHUNK_WIDTH + cx;

		for (int cy = 0; cy < Swan::CHUNK_HEIGHT; ++cy) {
			int tiley = chunk.pos_.y * Swan::CHUNK_HEIGHT + cy;

			Swan::TilePos pos(tilex, tiley);
			Swan::ChunkRelPos rel(cx, cy);
			chunk.setTileData(rel, genTile(pos));
		}
	}
}

void DefaultWorldGen::initializeChunk(const Swan::Context &ctx, Swan::Chunk &chunk)
{
	for (int cx = 0; cx < Swan::CHUNK_WIDTH; ++cx) {
		int tilex = chunk.pos_.x * Swan::CHUNK_WIDTH + cx;

		for (int cy = 0; cy < Swan::CHUNK_HEIGHT; ++cy) {
			int tiley = chunk.pos_.y * Swan::CHUNK_HEIGHT + cy;

			initializeTile(ctx, {tilex, tiley});
		}
	}
}

Swan::EntityRef DefaultWorldGen::spawnPlayer(const Swan::Context &ctx)
{
	int x = getPlayerX(perlin_);

	return ctx.plane.spawnEntity<PlayerEntity>(
		Swan::Vec2{(float)x, (float)getGrassLevel(perlin_, x) - 2});
}

}
