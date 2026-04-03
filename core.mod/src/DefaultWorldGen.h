#pragma once

#include <swan/swan.h>
#include <PerlinNoise.hpp>

#include "worldgen/Biome.h"
#include "worldgen/WGContext.h"

namespace CoreMod {

class DefaultWorldGen: public Swan::WorldGen {
public:
	static constexpr float DAY_LENGTH = 15 * 60;

	DefaultWorldGen(Swan::World &world, uint32_t seed):
		bgCave_(world.getSprite("core::misc/background-cave")),
		clouds_{
			world.getSprite("core::misc/cloud-1"),
			world.getSprite("core::misc/cloud-2"),
			world.getSprite("core::misc/cloud-3"),
		},
		wg_(seed, world)
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
	bool isOil(Swan::TilePos pos, int grassLevel);

	Swan::Tile::ID genTile(
		Swan::TilePos pos, const Biome &biome,
		int grassLevel, int stoneLevel);

	const Biome &getBiome(int x);

	Cygnet::RenderSprite bgCave_;
	Cygnet::RenderSprite clouds_[3];

	float time_ = 0;
	float timeOfDay_ = 0.3;
	float sunlightLevel_ = 1;

	WGContext wg_;
};

}
