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
	static constexpr float DAY_LENGTH = 15 * 60;

	DefaultWorldGen(Swan::World &world, uint32_t seed):
		seed_(seed),
		bgCave_(world.getSprite("core::misc/background-cave")),
		clouds_{
			world.getSprite("core::misc/cloud-1"),
			world.getSprite("core::misc/cloud-2"),
			world.getSprite("core::misc/cloud-3"),
		},
		oreDef_(world, seed_),
		shrubberyDef_(world, seed_),
		tallGrassDef_(world, seed_),
		treeDef_(world, seed_)
	{}

	void setTimeOfDay(float time) { timeOfDay_ = time; }
	float timeOfDay() { return timeOfDay_; }

	void drawBackground(
		Swan::Ctx &ctx, Cygnet::Renderer &rnd, Swan::Vec2 pos) override;
	Cygnet::Color backgroundColor(Swan::Vec2 pos) override;
	void genChunk(Swan::WorldPlane &plane, Swan::Chunk &chunk) override;
	Swan::EntityRef spawnPlayer(Swan::Ctx &ctx) override;
	void update(Swan::Ctx &ctx, float dt) override;
	void debugInfo() override;

	void serialize(Swan::Ctx &ctx, capnp::MessageBuilder &mb) override;
	void deserialize(Swan::Ctx &ctx, capnp::MessageReader &mr) override;

private:
	void drawSurfaceBackground(
		Swan::Ctx &ctx, Cygnet::Renderer &rnd, Swan::Vec2 pos, float factor);
	void drawCaveBackground(
		Swan::Ctx &ctx, Cygnet::Renderer &rnd, Swan::Vec2 pos, float factor);

	bool isCave(Swan::TilePos pos, int grassLevel);
	bool isLake(Swan::TilePos pos, int grassLevel, int stoneLevel);
	bool isOil(Swan::TilePos pos, int grassLevel);
	bool isClay(Swan::TilePos pos, int grassLevel, int stoneLevel);

	Swan::Tile::ID genTile(Swan::TilePos pos, int grassLevel, int stoneLevel);
	void initializeTile(Swan::Ctx &ctx, Swan::TilePos pos);

	const uint32_t seed_;
	Cygnet::RenderSprite bgCave_;
	Cygnet::RenderSprite clouds_[3];
	siv::PerlinNoise perlin_{seed_};

	OreDef oreDef_;
	ShrubberyDef shrubberyDef_;
	TallGrassDef tallGrassDef_;
	TreeDef treeDef_;
	float time_ = 0;
	float timeOfDay_ = 0.3;
	float sunlightLevel_ = 1;
};

}
