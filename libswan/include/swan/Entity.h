#pragma once

#include <memory>

#include "common.h"

namespace Swan {

class World;
class WorldPlane;
class Game;

class Entity {
public:
	class Factory {
	public:
		virtual ~Factory() = default;
		virtual Entity *create(const Context &ctx, const Vec2 &pos) = 0;
		std::string name_;
	};

	virtual ~Entity() = default;

	virtual const Vec2 &getPos() { return Vec2::ZERO; }

	virtual void draw(const Context &ctx, Win &win) {}
	virtual void update(const Context &ctx, float dt) {}
	virtual void tick() {}
};

}
