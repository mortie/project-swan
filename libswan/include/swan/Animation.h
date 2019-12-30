#pragma once

#include "common.h"
#include "Resource.h"
#include "Clock.h"
#include "Resource.h"

namespace Swan {

class Animation {
public:
	enum class Flags {
		HFLIP = 1,
	};

	Animation(ImageResource &resource, float interval, Flags flags = (Flags)0):
		resource_(resource), interval_(interval), timer_(interval), flags_(flags) {}

	void tick(float dt);
	void draw(Win &win);
	void reset();

private:
	ImageResource &resource_;
	float interval_;
	float timer_;
	Flags flags_;
	int frame_ = 0;
	bool dirty_ = true;
};

}
