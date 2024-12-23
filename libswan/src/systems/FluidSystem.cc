#include "systems/FluidSystem.h"

namespace Swan {

void FluidSystemImpl::tick()
{
	// Randomize update order
	for (size_t i = 1; i < updates_.size(); ++i) {
		size_t newIndex = random() % (updates_.size() - i) + i;
		if (i != newIndex) {
			std::swap(updates_[i], updates_[newIndex]);
		}
	}

	// Run the updates
	for (size_t i = 0; i < updates_.size(); ++i) {
		applyRules(updates_[i]);
	}
}

void FluidSystemImpl::applyRules(FluidPos pos)
{
}

}
