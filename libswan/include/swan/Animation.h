#pragma once

#include <SFML/Graphics/Sprite.hpp>

#include "common.h"
#include "Asset.h"
#include "Timer.h"

namespace Swan {

class Animation {
public:
	enum class Flags {
		HFLIP = 1,
	};

	Animation() = default;
	Animation(int w, int h, double interval, const Asset &asset, int flags = 0) {
		init(w, h, interval, asset, flags);
	}

	void init(int w, int h, double interval, const Asset &asset, int flags = 0);

	void tick(double dt);
	void draw(Win &win);
    void reset();

	int width_, height_;

private:
	double interval_;
	const Asset *asset_;
	int fcount_;
	int frame_ = 0;
	bool dirty_ = true;
	Timer timer_;
	sf::Sprite sprite_;
};

}
