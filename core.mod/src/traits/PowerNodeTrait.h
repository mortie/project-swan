#pragma once

#include "swan/EntityCollection.h"
#include <swan/swan.h>

namespace CoreMod {

class PowerNode {
public:
	PowerNode(Swan::Vec2 anchorPoint):
		anchorPoint_(anchorPoint)
	{}

	Swan::Vec2 anchorPoint()
	{ return anchorPoint_; }

	void onDespawn(Swan::Ctx &ctx)
	{
		for (auto &wire: wires_) {
			ctx.plane.entities().despawn(wire);
		}
	}

	void attach(Swan::EntityRef wire)
	{ wires_.push_back(wire); }

private:
	Swan::Vec2 anchorPoint_;
	std::vector<Swan::EntityRef> wires_;
};

class PowerNodeTrait {
public:
	struct Tag {};

	virtual PowerNode &get(Tag) = 0;

protected:
	~PowerNodeTrait() = default;
};

}
