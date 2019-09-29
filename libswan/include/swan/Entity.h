#pragma once

#include <memory>

#include "common.h"
#include "SRF.h"

namespace Swan {

class World;
class WorldPlane;
class Game;

class Entity {
public:
	class Factory {
	public:
		virtual ~Factory() = default;
		virtual Entity *create(const Context &ctx, const SRF &params) = 0;
		std::string name_;
	};

	virtual ~Entity() = default;

	virtual const Vec2 &getPos() { return Vec2::ZERO; }

	virtual void draw(const Context &ctx, Win &win) {}
	virtual void update(const Context &ctx, float dt) {}
	virtual void tick() {}
	virtual void readSRF(const Swan::Context &ctx, const SRF &srf) {}
	virtual SRF *writeSRF(const Swan::Context &ctx) { return new SRFNone(); }
};

}
