#include "DefaultWorldGen.h"

#include "entities/PlayerEntity.h"
#include "swan-common/constants.h"
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
	if (
		pos.y > grassLevel + 7 &&
		perlin_.noise2D(pos.x / 43.37, pos.y / 16.37) > 0.2) {
		return Swan::World::AIR_TILE_ID;
	}

	// Lakes
	if (
		pos.y >= grassLevel &&
		pos.y <= grassLevel + 10 &&
		perlin_.noise2D(pos.x / 20.6, pos.y / 14.565) > 0.4) {
		return tWater_;
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
	else{
		return tAir_;
	}
}

void DefaultWorldGen::genChunk(Swan::WorldPlane &plane, Swan::Chunk &chunk)
{
	Swan::TilePos chunkTilePos = chunk.pos() *
		Swan::TilePos{Swan::CHUNK_WIDTH, Swan::CHUNK_HEIGHT};

	constexpr int GEN_WIDTH = SwanCommon::CHUNK_WIDTH + GEN_PADDING * 2;
	constexpr int GEN_HEIGHT = SwanCommon::CHUNK_HEIGHT + GEN_PADDING * 2;

	StructureDef::Area area;

	area.begin = chunkTilePos.add(-GEN_PADDING, -GEN_PADDING);
	area.end = area.begin.add(GEN_WIDTH, GEN_HEIGHT);

	Swan::Tile::ID *rows[GEN_HEIGHT];
	area.rows = rows;

	Swan::Tile::ID buffer[GEN_WIDTH * GEN_HEIGHT];
	for (int ry = 0; ry < GEN_HEIGHT; ++ry) {
		rows[ry] = &buffer[ry * GEN_WIDTH];
	}

	for (int x = area.begin.x; x <= area.end.x; ++x) {
		int grassLevel = getGrassLevel(perlin_, x);
		int stoneLevel = getStoneLevel(perlin_, x);
		for (int y = area.begin.y; y <= area.end.y; ++y) {
			area({x, y}) = genTile({x, y}, grassLevel, stoneLevel);
		}
	}

	treeDef_.generateArea(area);
	tallGrassDef_.generateArea(area);

	for (int cy = 0; cy < Swan::CHUNK_HEIGHT; ++cy) {
		Swan::Tile::ID *crow = &chunk.getTileData()[cy * Swan::CHUNK_WIDTH];
		Swan::Tile::ID *arow = &buffer[(cy + GEN_PADDING) * GEN_WIDTH];
		memcpy(
			crow, &arow[GEN_PADDING],
			Swan::CHUNK_WIDTH * sizeof(Swan::Tile::ID));
	}
}

Swan::EntityRef DefaultWorldGen::spawnPlayer(const Swan::Context &ctx)
{
	int x = getPlayerX(perlin_);

	return ctx.plane.entities().spawn<PlayerEntity>(
		Swan::Vec2{(float)x, (float)getGrassLevel(perlin_, x) - 2});
}

}
