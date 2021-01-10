#pragma once

#include <SDL.h>
#include <cygnet/Renderer.h>

#include "common.h"
#include "Clock.h"

namespace Swan {

class Animation {
public:
	Animation(Cygnet::RenderSprite sprite, float interval, Cygnet::Mat3gf mat = {}):
		sprite_(sprite), interval_(interval), timer_(interval), mat_(mat) {}

	void tick(float dt);
	void draw(const Vec2 &pos, Cygnet::Renderer &rnd);
	void reset();

private:
	Cygnet::RenderSprite sprite_;
	float interval_;
	float timer_;
	Cygnet::Mat3gf mat_;
	int frame_ = 0;
};

}
