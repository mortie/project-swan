#pragma once

#include <memory>
#include <unordered_set>
#include <vector>
#include <string>
#include <span>
#include <cygnet/Renderer.h>
#include <cygnet/ResourceManager.h>
#include <cygnet/util.h>
#include <swan/log.h>

#include "Clock.h"
#include "Mod.h"
#include "WorldData.h"
#include "common.h"
#include "WorldPlane.h"
#include "assets.h"
#include "swan.capnp.h"
#include "multiplayer.capnp.h"

namespace Swan {

class Game;

class World {
public:
	struct TickProgress {
		bool ongoing = false;
		std::unordered_set<WorldPlane::ID> tickedPlanes;
	};

	World(Game *game, uint32_t seed, std::span<const std::string> modPaths);

	void setWorldGen(std::string gen);
	void spawnPlayer();

	void setCurrentPlane(WorldPlane &plane);

	WorldPlane &currentPlane()
	{
		return *planes_[currentPlane_].plane;
	}

	WorldPlane &addPlane(std::string gen);

	WorldPlane &addPlane()
	{
		return addPlane(defaultWorldGen_);
	}

	Cygnet::Color backgroundColor();
	void draw(Cygnet::Renderer &rnd);
	void update(float dt);
	bool tick(float dt, RTDeadline deadline);

	uint32_t seed() const { return seed_; }

	void serialize(proto::World::Builder w);
	void deserialize(proto::World::Reader r);

	WorldData &data() { return data_; }

	EntityRef playerRef_;
	Body *player_;

private:
	class ChunkRenderer {
	public:
		void tick(WorldPlane &plane, ChunkPos abspos);
	};

	struct PlaneWrapper {
		std::string worldGen;
		std::unique_ptr<WorldPlane> plane;
	};

	std::vector<ModWrapper> loadMods(std::span<const std::string> paths);
	void buildResources();

	Game *game_;

	int resourceTickCounter_ = 0;

	uint32_t seed_;
	ChunkRenderer chunkRenderer_;
	WorldPlane::ID currentPlane_ = 0;
	std::vector<PlaneWrapper> planes_;
	std::string defaultWorldGen_;
	TickProgress tickProgress_;
	WorldData data_;
};

}
