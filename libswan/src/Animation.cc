#include "Animation.h"

#include "Win.h"
#include "gfxutil.h"

namespace Swan {

void Animation::tick(float dt) {
	timer_ -= dt;
	if (timer_ <= 0) {
		timer_ += interval_;

		frame_ += 1;
		if (frame_ >= resource_.numFrames_)
			frame_ = 0;
	}
}

void Animation::draw(const Vec2 &pos, Win &win) {
	SDL_Rect rect = resource_.frameRect(frame_);
	win.showTexture(pos, resource_.texture_.get(), &rect, { .flip = flip_ });
}

void Animation::reset() {
	timer_ = interval_;
	frame_ = 0;
}

}
