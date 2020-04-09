#pragma once

#include <swan/swan.h>

#include <PerlinNoise/PerlinNoise.hpp>

class DefaultWorldGen: public Swan::WorldGen {
public:
	DefaultWorldGen(Swan::World &world):
		tGrass_(world.getTileID("core::grass")),
		tDirt_(world.getTileID("core::dirt")),
		tStone_(world.getTileID("core::stone")),
		tAir_(world.getTileID("@::air")),
		tTreeTrunk_(world.getTileID("core::tree-trunk")),
		tLeaves_(world.getTileID("core::leaves")),
		bgCave_(world.resources_.getImage("core/misc/background-cave")) {}

	void drawBackground(const Swan::Context &ctx, Swan::Win &win, Swan::Vec2 pos) override;
	SDL_Color backgroundColor(Swan::Vec2 pos) override;
	void genChunk(Swan::WorldPlane &plane, Swan::Chunk &chunk) override;
	Swan::BodyTrait::HasBody *spawnPlayer(const Swan::Context &ctx) override;

private:
	Swan::Tile::ID genTile(Swan::TilePos pos);
	Swan::Tile::ID tGrass_, tDirt_, tStone_, tAir_, tTreeTrunk_, tLeaves_;
	Swan::ImageResource &bgCave_;
	siv::PerlinNoise perlin_ = siv::PerlinNoise(100);
};
