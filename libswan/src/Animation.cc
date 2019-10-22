#include "Animation.h"

#include "Win.h"

namespace Swan {

void Animation::init(int w, int h, float interval, const Asset &asset, int flags) {
	width_ = w;
	height_ = h;
	interval_ = interval;
	asset_ = &asset;
	fcount_ = (int)asset_->image_.getSize().y / height_;
	sprite_.setTexture(asset_->tex_);
	sprite_.setTextureRect(sf::IntRect(0, 0, width_, height_));

	if (flags & (int)Flags::HFLIP) {
		sprite_.setOrigin(Vec2(width_, 0));
		sprite_.setScale(Vec2(-1, 1));
	}
}

void Animation::tick(float dt) {
	timer_.tick(dt);
	if (timer_.periodic(interval_)) {
		dirty_ = true;
		frame_ += 1;
		if (frame_ >= fcount_)
			frame_ = 0;

		sprite_.setTextureRect(sf::IntRect(0, height_ * frame_, width_, height_));
	}
}

void Animation::draw(Win &win) {
	win.draw(sprite_);
}

void Animation::reset() {
	timer_.reset();
    frame_ = 0;
    dirty_ = true;
}

}
