#include "Animation.h"

namespace Swan {

Animation::Animation(int w, int h, double freq, const sf::Image &img):
		width_(w), height_(h), img_(img) {
	fcount_ = img_.getSize().y / height_;
	interval_ = 1.0 / freq;
}

Animation::Animation(int w, int h, double freq, const std::string &path):
		width_(w), height_(h) {
	img_.loadFromFile(path);
	fcount_ = img_.getSize().y / height_;
	interval_ = 1.0 / freq;
}

void Animation::tick(double dt) {
	if (time_ > interval_) {
		dirty_ = true;
		frame_ += 1;
		time_ = 0;
		if (frame_ >= fcount_)
			frame_ = 0;
	}

	time_ += dt;
}

bool Animation::fill(sf::Texture &tex, bool force) {
	if (!force && !dirty_)
		return false;

	const sf::Uint8 *data = img_.getPixelsPtr() + 4 * width_ * height_ * frame_;
	tex.update(data);
	dirty_ = false;
	return true;
}

}
