#pragma once

#include <memory>

#include "common.h"
#include "WorldPlane.h"

namespace Swan {

class Entity {
public:
	class Factory {
	public:
		std::string name_;
		virtual Entity *create(const Vec2 &pos) = 0;
		virtual ~Factory() = default;
	};

	virtual ~Entity() = default;

	virtual void draw(Win &win) {}
	virtual void update(WorldPlane &plane, float dt) {}
	virtual void tick() {}
};

}
