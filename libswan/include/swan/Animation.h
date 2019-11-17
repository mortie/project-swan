#pragma once

#include "common.h"
#include "Resource.h"
#include "Timer.h"
#include "Resource.h"

namespace Swan {

class Animation {
public:
	enum class Flags {
		HFLIP = 1,
	};

	Animation(ImageResource &resource, float interval):
		resource_(resource), interval_(interval), timer_(interval) {}

	void tick(float dt);
	void draw(Win &win);
	void reset();

private:
	ImageResource &resource_;
	float interval_;
	float timer_;
	int frame_ = 0;
	bool dirty_ = true;
};

}
