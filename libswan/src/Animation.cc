#include "Animation.h"

#include "Win.h"

namespace Swan {

void Animation::tick(float dt) {
	timer_ -= dt;
	if (timer_ <= 0) {
		timer_ += interval_;

		frame_ += 1;
		if (frame_ >= resource_.num_frames_)
			frame_ = 0;

		dirty_ = true;
	}
}

void Animation::draw(Win &win) {
}

void Animation::reset() {
	timer_ = interval_;
	frame_ = 0;
	dirty_ = true;
}

}
