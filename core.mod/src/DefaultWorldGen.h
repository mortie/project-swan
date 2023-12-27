#pragma once

#include <swan/swan.h>

#include <PerlinNoise.hpp>

namespace CoreMod {

class DefaultWorldGen: public Swan::WorldGen {
public:
	DefaultWorldGen(Swan::World &world):
		tGrass_(world.getTileID("core::grass")),
		tDirt_(world.getTileID("core::dirt")),
		tStone_(world.getTileID("core::stone")),
		tTreeSeeder_(world.getTileID("core::tree-seeder")),
		tAir_(world.getTileID("@::air")),
		bgCave_(world.getSprite("core::misc/background-cave")) {}

	void drawBackground(
			const Swan::Context &ctx, Cygnet::Renderer &rnd, Swan::Vec2 pos) override;
	Cygnet::Color backgroundColor(Swan::Vec2 pos) override;
	void genChunk(Swan::WorldPlane &plane, Swan::Chunk &chunk) override;
	void initializeChunk(const Swan::Context &plane, Swan::Chunk &chunk) override;
	Swan::EntityRef spawnPlayer(const Swan::Context &ctx) override;

private:
	Swan::Tile::ID genTile(Swan::TilePos pos);
	void initializeTile(const Swan::Context &ctx, Swan::TilePos pos);

	const uint32_t seed_ = 100;
	Swan::Tile::ID tGrass_, tDirt_, tStone_, tTreeSeeder_, tAir_;
	Cygnet::RenderSprite bgCave_;
	siv::PerlinNoise perlin_{seed_};
};

}
