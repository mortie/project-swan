#include "DefaultWorldGen.h"

#include "entities/PlayerEntity.h"
#include "swan/constants.h"
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
	float opacity = 1;
	float y = pos.y;
	if (y < 15) {
		return;
	}

	if (y < 35) {
		opacity = (y - 15) / (35 - 15);
	}

	pos -= {2.5, 2.5};
	pos /= 8;
	Swan::Vec2i whole = pos.as<int>();
	Swan::Vec2 frac = pos - whole.as<float>();

	pos = whole.as<float>() * 8 + frac * 4;
	for (int y = -10; y <= 10; ++y) {
		for (int x = -15; x <= 15; ++x) {
			rnd.drawSprite(Cygnet::RenderLayer::BACKGROUND, {
				.transform = Cygnet::Mat3gf{}.translate(pos.add(x * 4, y * 4)),
				.sprite = bgCave_,
				.opacity = opacity,
			});
		}
	}
}

Cygnet::Color DefaultWorldGen::backgroundColor(Swan::Vec2 pos)
{
	float y = pos.y;

	return Swan::UI::linearGradient(y, {
		{0, Cygnet::ByteColor{128, 220, 250}},
		{70, Cygnet::ByteColor{107, 87, 5}},
		{100, Cygnet::ByteColor{107, 87, 5}},
		{200, Cygnet::ByteColor{20, 20, 23}},
		{300, Cygnet::ByteColor{20, 20, 23}},
		{500, Cygnet::ByteColor{25, 10, 10}},
		{1000, Cygnet::ByteColor{65, 10, 10}},
	});
}

bool DefaultWorldGen::isCave(Swan::TilePos pos, int grassLevel)
{
	return
		pos.y > grassLevel + 7 &&
		perlin_.noise2D(pos.x / 43.37, pos.y / 16.37) > 0.2;
}

bool DefaultWorldGen::isLake(Swan::TilePos pos, int grassLevel)
{
	return
		pos.y >= grassLevel &&
		pos.y <= grassLevel + 10 &&
		perlin_.noise2D(pos.x / 20.6, pos.y / 14.565) > 0.4;
}

bool DefaultWorldGen::isOil(Swan::TilePos pos, int grassLevel)
{
	return
		pos.y > grassLevel + 30 &&
		pos.y <= grassLevel + 500 &&
		perlin_.noise2D(pos.x / 70.6, pos.y / 40.565) > 0.4;
}

Swan::Tile::ID DefaultWorldGen::genTile(
	Swan::TilePos pos,
	int grassLevel,
	int stoneLevel)
{
	// Oil lakes leading into caves tanks performance,
	// so only produce cave air if we're not next to oil
	bool spawnCave =
		isCave(pos, grassLevel) &&
		!isOil(pos.add(-1, 0), grassLevel) &&
		!isOil(pos.add(1, 0), grassLevel) &&
		!isOil(pos.add(0, -1), grassLevel) &&
		!isOil(pos.add(0, 1), grassLevel);
	if (spawnCave) {
		return tAir_;
	}

	if (isLake(pos, grassLevel)) {
		return tWater_;
	}

	if (isOil(pos, grassLevel)) {
		return tOil_;
	}

	if (pos.y > stoneLevel) {
		return tStone_;
	}
	if (pos.y > grassLevel) {
		return tDirt_;
	}
	if (pos.y == grassLevel) {
		return tGrass_;
	}
	else {
		return tAir_;
	}
}

void DefaultWorldGen::genChunk(Swan::WorldPlane &plane, Swan::Chunk &chunk)
{
	Swan::TilePos chunkTilePos = chunk.pos() *
		Swan::TilePos{Swan::CHUNK_WIDTH, Swan::CHUNK_HEIGHT};

	constexpr int GEN_WIDTH = Swan::CHUNK_WIDTH + GEN_PADDING * 2;
	constexpr int GEN_HEIGHT = Swan::CHUNK_HEIGHT + GEN_PADDING * 2;

	StructureDef::Area area;

	area.begin = chunkTilePos.add(-GEN_PADDING, -GEN_PADDING);
	area.end = area.begin.add(GEN_WIDTH, GEN_HEIGHT);

	Swan::Tile::ID *rows[GEN_HEIGHT];
	area.rows = rows;

	Swan::Tile::ID buffer[GEN_WIDTH * GEN_HEIGHT];
	for (int ry = 0; ry < GEN_HEIGHT; ++ry) {
		rows[ry] = &buffer[ry * GEN_WIDTH];
	}

	int surfaceLevels[GEN_WIDTH];
	area.surfaceLevels = surfaceLevels;

	for (int x = area.begin.x; x <= area.end.x; ++x) {
		int grassLevel = getGrassLevel(perlin_, x);
		area.surfaceLevel(x) = grassLevel;

		if (grassLevel >= area.begin.y && grassLevel < area.end.y) {
			area.hasSurface = true;
		}

		int stoneLevel = getStoneLevel(perlin_, x);
		for (int y = area.begin.y; y <= area.end.y; ++y) {
			area({x, y}) = genTile({x, y}, grassLevel, stoneLevel);
		}
	}

	oreDef_.generateArea(area);
	shrubberyDef_.generateArea(area);
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
