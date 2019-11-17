#pragma once

#include "common.h"

#include <SDL2/SDL.h>

namespace Swan {

class Win {
public:
	Win(SDL_Renderer *renderer): renderer_(renderer) {}

	void setPos(const Vec2 &pos) {
		//transform_ = sf::Transform()
		//	.scale(scale_, scale_)
		//	.translate((pos - cam_) * TILE_SIZE);
	}

	Vec2 getSize() {
		//sf::Vector2u v = window_->getSize();
		//return Vec2(v.x, v.y) / (TILE_SIZE * scale_);
		return Vec2(10, 10);
	}

	float scale_ = 2;
	Vec2 cam_;
	SDL_Renderer *renderer_;
};

}
