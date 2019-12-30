#pragma once

#include "log.h"
#include "common.h"

#include <SDL2/SDL.h>

namespace Swan {

class Win {
public:
	Win(SDL_Renderer *renderer): renderer_(renderer) {
		if (SDL_GetRendererInfo(renderer_, &rinfo_) < 0) {
			panic << "GetRenedrerInfo failed: " << SDL_GetError();
			abort();
		}

		info << "Using renderer: " << rinfo_.name;
	}

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

	void showTexture(const Vec2 &pos, SDL_Texture *tex, SDL_Rect *srcrect) {
		SDL_Rect destrect{
			(int)pos.x * TILE_SIZE, (int)pos.y * TILE_SIZE,
			srcrect->w, srcrect->h,
		};

		if (SDL_RenderCopy(renderer_, tex, srcrect, &destrect) < 0) {
			panic << "RenderCopy failed: " << SDL_GetError();
			abort();
		}
	}

	float scale_ = 2;
	Vec2 cam_;
	SDL_Renderer *renderer_;
	SDL_RendererInfo rinfo_;
};

}
