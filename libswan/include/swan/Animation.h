#pragma once

#include <cygnet/Renderer.h>

#include "common.h"

namespace Swan {

class Animation {
public:
	Animation(Cygnet::RenderSprite sprite, float interval, int loopFrom = 0, Cygnet::Mat3gf mat = {}):
		sprite_(sprite), interval_(interval), timer_(interval)
	{}

	void tick(float dt);
	void reset();
	void draw(Cygnet::Renderer &rnd, Cygnet::Mat3gf mat);

	bool done()
	{
		return done_;
	}

	void setInterval(float interval)
	{
		interval_ = interval;
	}

private:
	Cygnet::RenderSprite sprite_;
	float interval_;
	float timer_;
	int frame_ = 0;
	bool done_ = false;
};

inline void Animation::draw(Cygnet::Renderer &rnd, Cygnet::Mat3gf mat)
{
	rnd.drawSprite({mat, frame_, sprite_});
}

}
