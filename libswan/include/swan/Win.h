#pragma once

#include "log.h"
#include "common.h"

#include <SDL2/SDL.h>

namespace Swan {

class Win {
public:
	Win(SDL_Window *window, SDL_Renderer *renderer): window_(window), renderer_(renderer) {
		if (SDL_GetRendererInfo(renderer_, &rinfo_) < 0) {
			panic << "GetRenedrerInfo failed: " << SDL_GetError();
			abort();
		}

		info << "Using renderer: " << rinfo_.name;
	}

	Vec2 getSize() {
		int w, h;
		SDL_GetWindowSize(window_, &w, &h);
		return Vec2((float)w / TILE_SIZE, (float)h / TILE_SIZE);
	}

	void showTexture(
			const Vec2 &pos, SDL_Texture *tex, SDL_Rect *srcrect,
			SDL_RendererFlip flip = SDL_FLIP_NONE, double angle = 0) {

		SDL_Rect destrect{
			(int)((pos.x - cam_.x) * TILE_SIZE), (int)((pos.y - cam_.y) * TILE_SIZE),
			srcrect->w, srcrect->h,
		};

		if (SDL_RenderCopyEx(renderer_, tex, srcrect, &destrect, angle, NULL, flip) < 0) {
			panic << "RenderCopy failed: " << SDL_GetError();
			abort();
		}
	}

	float scale_ = 2;
	Vec2 cam_;
	SDL_Window *window_;
	SDL_Renderer *renderer_;
	SDL_RendererInfo rinfo_;
};

}
