#pragma once

#include <memory>

#include "common.h"

namespace Swan {

class WorldPlane;

class Entity {
public:
	class Factory {
	public:
		virtual ~Factory() = default;
		virtual Entity *create(const Vec2 &pos) = 0;
		std::string name_;
	};

	virtual ~Entity() = default;

	virtual void draw(Win &win) {}
	virtual void update(WorldPlane &plane, float dt) {}
	virtual void tick() {}
};

}
