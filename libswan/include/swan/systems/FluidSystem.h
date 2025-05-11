#pragma once

#include "../FastHashSet.h"
#include "../common.h"
#include "../Fluid.h"
#include "../Clock.h"
#include "swan.capnp.h"

#include <cygnet/util.h>
#include <cstdint>
#include <unordered_set>
#include <vector>

namespace Cygnet {
class Renderer;
}

namespace Swan {

class WorldPlane;

class FluidSystemImpl {
public:
	FluidSystemImpl(WorldPlane &plane): plane_(plane) {}

	struct TickProgress {
		size_t updateIndex = 0;
	};

	/*
	 * Available to game logic
	 */

	void triggerUpdateInTile(TilePos pos);
	void setInTile(TilePos pos, Fluid::ID fluid);
	void replaceInTile(TilePos pos, Fluid::ID fluid);
	int numUpdates() { return updatesB_.size(); }
	int numParticles() { return particles_.size(); }
	Fluid &getAtPos(Vec2 pos);

	/*
	 * Available to friends
	 */

	void draw(Cygnet::Renderer &rnd);
	void update(float dt);
	bool tick(RTDeadline deadline);

	void serialize(proto::FluidSystem::Builder w);
	void deserialize(proto::FluidSystem::Reader r);

private:
	struct FluidParticle {
		Vec2 pos;
		Vec2 vel;
		Cygnet::Color color;
		Fluid::ID id;
		uint8_t remainingTime;
	};

	class FluidCellRef {
	public:
		FluidCellRef(uint8_t *value): value_(value) {}

		void setAir();
		bool isAir();
		bool isSolid();
		void set(Fluid::ID id, int vx);
		int vx();
		void setVX(int vx);
		Fluid::ID id();
		void setID(Fluid::ID id);

	private:
		uint8_t *value_;
	};

	void triggerUpdate(FluidPos pos);
	void triggerUpdateAround(FluidPos pos);

	void applyRules(FluidPos pos);
	FluidCellRef getFluidCell(FluidPos pos);

	WorldPlane &plane_;

	std::unordered_set<FluidPos> updateSet_;
	std::unordered_set<FluidPos> movedSet_;
	std::vector<FluidPos> updatesA_;
	std::vector<FluidPos> updatesB_;
	std::vector<FluidParticle> particles_;
	TickProgress tickProgress_;
};

class FluidSystem: private FluidSystemImpl {
public:
	using FluidSystemImpl::FluidSystemImpl;

	using FluidSystemImpl::triggerUpdateInTile;
	using FluidSystemImpl::setInTile;
	using FluidSystemImpl::replaceInTile;
	using FluidSystemImpl::numUpdates;
	using FluidSystemImpl::numParticles;
	using FluidSystemImpl::getAtPos;

	friend WorldPlane;
};

}
