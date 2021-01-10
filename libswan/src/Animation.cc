#include "Animation.h"

#include <cygnet/Renderer.h>

namespace Swan {

void Animation::tick(float dt) {
	timer_ -= dt;
	if (timer_ <= 0) {
		timer_ += interval_;

		frame_ += 1;
		if (frame_ >= sprite_.frameCount)
			frame_ = 0;
	}
}

void Animation::draw(const Vec2 &pos, Cygnet::Renderer &rnd) {
	rnd.drawSprite(sprite_, mat_, frame_);
}

void Animation::reset() {
	timer_ = interval_;
	frame_ = 0;
}

}
