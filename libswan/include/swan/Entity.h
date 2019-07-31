#pragma once

#include <memory>

#include "common.h"

namespace Swan {

class World;
class WorldPlane;

class Entity {
public:
	class Factory {
	public:
		virtual ~Factory() = default;
		virtual Entity *create(World &world, const Vec2 &pos) = 0;
		std::string name_;
	};

	virtual ~Entity() = default;

	virtual const Vec2 &getPos() { return Vec2::ZERO; }

	virtual void draw(Win &win) {}
	virtual void update(WorldPlane &plane, float dt) {}
	virtual void tick() {}
};

}
