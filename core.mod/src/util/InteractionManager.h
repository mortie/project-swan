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

	virtual void update(Swan::Ctx &ctx, const Info &info)
	{}

	virtual void activate(Swan::Ctx &ctx, const Info &info)
	{}

	virtual void draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd)
	{}

	virtual void destroy(Swan::Ctx &ctx)
	{}

	bool done()
	{ return done_; }

protected:
	bool done_ = false;
};

}
