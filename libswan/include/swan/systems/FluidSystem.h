#pragma once

#include "../FastHashSet.h"
#include "../common.h"
#include "cygnet/util.h"

#include <vector>

namespace Swan {

class WorldPlane;

class FluidSystemImpl {
public:
	FluidSystemImpl(WorldPlane &plane): plane_(plane) {}

	/*
	 * Available to friends
	 */

	void triggerUpdate(FluidPos pos);
	void tick();

private:
	struct FluidParticle {
		Vec2 velocity;
		Cygnet::Color color;
	};

	void applyRules(FluidPos pos);

	WorldPlane &plane_;

	FastHashSet<FluidPos> updateSet_;
	std::vector<FluidPos> updates_;
	std::vector<FluidParticle> particles_;
};

class FluidSystem: private FluidSystemImpl {
public:
	using FluidSystemImpl::FluidSystemImpl;

	friend WorldPlane;
};

}
