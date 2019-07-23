#pragma once

#include <memory>

#include "common.h"

namespace Swan {

class Entity {
public:
	class Factory {
		public:
			std::string name_;
			virtual Entity *create(const Vec2 &pos) = 0;
	};

	virtual void draw(Win &win) {}
	virtual void update(float dt) {}
	virtual void tick() {}
};

}
