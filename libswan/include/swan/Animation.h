#pragma once

#include <SDL.h>

#include "common.h"
#include "Resource.h"
#include "Clock.h"
#include "Resource.h"

namespace Swan {

class Animation {
public:
	Animation(ImageResource &resource, float interval, SDL_RendererFlip flip = SDL_FLIP_NONE):
		resource_(resource), interval_(interval), timer_(interval), flip_(flip) {}

	void tick(float dt);
	void draw(const Vec2 &pos, Win &win);
	void reset();

private:
	ImageResource &resource_;
	float interval_;
	float timer_;
	SDL_RendererFlip flip_;
	int frame_ = 0;
};

}
