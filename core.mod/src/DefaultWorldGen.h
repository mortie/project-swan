#pragma once

#include <swan/swan.h>
#include <PerlinNoise.hpp>

#include "worldgen/OreDef.h"
#include "worldgen/ShrubberyDef.h"
#include "worldgen/TallGrassDef.h"
#include "worldgen/TreeDef.h"

namespace CoreMod {

class DefaultWorldGen: public Swan::WorldGen {
public:
	DefaultWorldGen(Swan::World &world):
		tGrass_(world.getTileID("core::grass")),
		tDirt_(world.getTileID("core::dirt")),
		tStone_(world.getTileID("core::stone")),
		tWater_(world.getTileID("core::water")),
		tAir_(world.getTileID("@::air")),
		bgCave_(world.getSprite("core::misc/background-cave")),
		oreDef_(world, seed_),
		shrubberyDef_(world, seed_),
		tallGrassDef_(world, seed_),
		treeDef_(world, seed_)
	{}

	void drawBackground(
		const Swan::Context &ctx, Cygnet::Renderer &rnd, Swan::Vec2 pos) override;
	Cygnet::Color backgroundColor(Swan::Vec2 pos) override;
	void genChunk(Swan::WorldPlane &plane, Swan::Chunk &chunk) override;
	Swan::EntityRef spawnPlayer(const Swan::Context &ctx) override;

private:
	Swan::Tile::ID genTile(Swan::TilePos pos, int grassLevel, int stoneLevel);
	void initializeTile(const Swan::Context &ctx, Swan::TilePos pos);

	const uint32_t seed_ = 100;
	Swan::Tile::ID tGrass_, tDirt_, tStone_, tWater_, tAir_;
	Cygnet::RenderSprite bgCave_;
	siv::PerlinNoise perlin_{seed_};

	OreDef oreDef_;
	ShrubberyDef shrubberyDef_;
	TallGrassDef tallGrassDef_;
	TreeDef treeDef_;
};

}
