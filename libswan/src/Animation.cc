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

void Animation::reset() {
	timer_ = interval_;
	frame_ = 0;
}

}
