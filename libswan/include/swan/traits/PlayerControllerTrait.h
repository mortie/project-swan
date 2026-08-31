#pragma once

#include <cygnet/Renderer.h>
#include "../common.h"

namespace Swan {

class PlayerControllerTrait {
public:
	virtual void controlPlayer(Ctx &ctx, float dt) = 0;
	virtual void drawUI(Ctx &ctx, Cygnet::Renderer &rnd) = 0;

protected:
	virtual ~PlayerControllerTrait() = default;
};

}
