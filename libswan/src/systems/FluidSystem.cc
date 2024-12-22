#include "systems/FluidSystem.h"

namespace Swan {

void FluidSystem::tick()
{
	// Randomize update order
	for (size_t i = 1; i < updates_.size(); ++i) {
		size_t newIndex = random() % (updates_.size() - i) + i;
		if (i != newIndex) {
			std::swap(updates_[i], updates_[newIndex]);
		}
	}

	// Apply rules
	for (size_t i = 0; i < updates_.size(); ++i) {
		applyRules(updates_[i]);
	}
}

void FluidSystem::applyRules(FluidPos pos)
{
}

}
