#pragma once

#include "entities/CopperWireEntity.h"
#include "swan/Entity.h"
#include "swan/EntityCollection.h"
#include <optional>
#include <swan/swan.h>
#include <type_traits>
#include <vector>
#include <unordered_set>

namespace CoreMod {

class PowerNode {
public:
	PowerNode(Swan::Vec2 anchorPoint):
		anchorPoint_(anchorPoint)
	{}

	Swan::Vec2 anchorPoint()
	{ return anchorPoint_; }

	void onDespawn(Swan::Ctx &ctx);
	void attach(Swan::EntityRef wire);

	Swan::EntityRef powerSource();
	void invalidateNetwork();

private:
	Swan::EntityRef findPowerSource(std::unordered_set<PowerNode *> &seen);
	void invalidateNetwork(std::unordered_set<PowerNode *> &seen);

	Swan::Vec2 anchorPoint_;
	std::vector<Swan::EntityRef> wires_;

	// For performance reasons, we cache the power source
	// so that we don't have to search for it more than necessary.
	// Here, nullopt means that we need to re-check,
	// a null entity means that we know that there's no power source.
	std::optional<Swan::EntityRef> powerSource_;
};

class PowerNodeTrait {
public:
	struct Tag {};

	virtual PowerNode &get(Tag) = 0;

protected:
	~PowerNodeTrait() = default;
};

}
