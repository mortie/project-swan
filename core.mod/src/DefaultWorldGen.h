#pragma once

#include <swan/swan.h>
#include <PerlinNoise.hpp>

#include "worldgen/defs/OreDef.h"
#include "worldgen/defs/ShrubberyDef.h"
#include "worldgen/defs/TallGrassDef.h"
#include "worldgen/defs/TreeDef.h"

namespace CoreMod {

class DefaultWorldGen: public Swan::WorldGen {
public:
	DefaultWorldGen(Swan::World &world):
		tGrass_(world.getTileID("core::grass")),
		tDirt_(world.getTileID("core::dirt")),
		tStone_(world.getTileID("core::stone")),
		tClay_(world.getTileID("core::clay-tile")),
		tWater_(world.getTileID("core::water")),
		tOil_(world.getTileID("core::oil")),
		tAir_(world.getTileID("@::air")),
		bgCave_(world.getSprite("core::misc/background-cave")),
		oreDef_(world, seed_),
		shrubberyDef_(world, seed_),
		tallGrassDef_(world, seed_),
		treeDef_(world, seed_)
	{}

	void drawBackground(
		Swan::Ctx &ctx, Cygnet::Renderer &rnd, Swan::Vec2 pos) override;
	Cygnet::Color backgroundColor(Swan::Vec2 pos) override;
	void genChunk(Swan::WorldPlane &plane, Swan::Chunk &chunk) override;
	Swan::EntityRef spawnPlayer(Swan::Ctx &ctx) override;

private:
	bool isCave(Swan::TilePos pos, int grassLevel);
	bool isLake(Swan::TilePos pos, int grassLevel, int stoneLevel);
	bool isOil(Swan::TilePos pos, int grassLevel);
	bool isClay(Swan::TilePos pos, int grassLevel, int stoneLevel);

	Swan::Tile::ID genTile(Swan::TilePos pos, int grassLevel, int stoneLevel);
	void initializeTile(Swan::Ctx &ctx, Swan::TilePos pos);

	const uint32_t seed_ = 100;
	Swan::Tile::ID tGrass_, tDirt_, tStone_, tClay_, tWater_, tOil_, tAir_;
	Cygnet::RenderSprite bgCave_;
	siv::PerlinNoise perlin_{seed_};

	OreDef oreDef_;
	ShrubberyDef shrubberyDef_;
	TallGrassDef tallGrassDef_;
	TreeDef treeDef_;
};

}
