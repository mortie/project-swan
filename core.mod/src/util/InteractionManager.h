#pragma once

#include "swan/common.h"
#include <swan/swan.h>

namespace CoreMod {

class InteractionManager {
public:
	virtual ~InteractionManager() = default;

	struct Info {
		Swan::Vec2 lookPos;
		Swan::TilePos breakPos;
		Swan::TilePos placePos;
	};

	virtual void update(Swan::Ctx &ctx, float dt, const Info &info)
	{}

	virtual bool activate(Swan::Ctx &ctx, const Info &info)
	{ return false; }

	virtual void draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd)
	{}

	bool done()
	{ return done_; }

protected:
	bool done_ = false;
};

}
