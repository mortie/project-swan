#pragma once

#include "log.h"
#include "common.h"

#include <SDL2/SDL.h>
#include <optional>

namespace Swan {

class Win {
public:
	Win(SDL_Window *window, SDL_Renderer *renderer, float scale):
			window_(window), renderer_(renderer), scale_(scale) {

		if (SDL_GetRendererInfo(renderer_, &rinfo_) < 0) {
			panic << "GetRenedrerInfo failed: " << SDL_GetError();
			abort();
		}

		// For HiDPI, we must set the renderer's logical size.
		int w, h;
		SDL_GetWindowSize(window_, &w, &h);
		onResize(w, h);

		info << "Using renderer: " << rinfo_.name;
	}

	Vec2 getPixSize() {
		int w, h;
		SDL_GetWindowSize(window_, &w, &h);
		return Vec2((float)w / scale_, (float)h / scale_);
	}

	Vec2 getSize() {
		int w, h;
		SDL_GetWindowSize(window_, &w, &h);
		return Vec2(((float)w / (scale_ * zoom_)) / TILE_SIZE, ((float)h / (scale_ * zoom_)) / TILE_SIZE);
	}

	void onResize(int w, int h) {
		SDL_RenderSetLogicalSize(renderer_, (int)((float)w / scale_), (int)((float)h / scale_));
	}

	SDL_Rect createDestRect(Vec2 pos, Vec2 pixsize) {
		return SDL_Rect{
			(int)((pos.x - cam_.x) * TILE_SIZE * zoom_),
			(int)((pos.y - cam_.y) * TILE_SIZE * zoom_),
			(int)(pixsize.x * zoom_), (int)(pixsize.y * zoom_),
		};
	}

	struct ShowTextureArgs {
		SDL_RendererFlip flip = SDL_FLIP_NONE;
		double hscale = 1;
		double vscale = 1;
		double angle = 0;
		std::optional<SDL_Point> center = std::nullopt;
	};

	void showTexture(
			const Vec2 &pos, SDL_Texture *tex, SDL_Rect *srcrect,
			ShowTextureArgs args) {

		SDL_Point *center = args.center ? &*args.center : nullptr;
		SDL_Rect destrect = createDestRect(pos, Vec2(srcrect->w * args.hscale, srcrect->h * args.hscale));
		if (SDL_RenderCopyEx(renderer_, tex, srcrect, &destrect, args.angle, center, args.flip) < 0)
			warn << "RenderCopyEx failed: " << SDL_GetError();
	}

	// We want an overload which uses RenderCopy instead of RenderCopyEx,
	// because RenderCopy might be faster
	void showTexture(const Vec2 &pos, SDL_Texture *tex, SDL_Rect *srcrect) {
		SDL_Rect destrect = createDestRect(pos, Vec2(srcrect->w, srcrect->h));
		if (SDL_RenderCopy(renderer_, tex, srcrect, &destrect) < 0)
			warn << "RenderCopy failed: " << SDL_GetError();
	}

	void drawRect(const Vec2 &pos, const Vec2 &size) {
		SDL_Rect destrect = createDestRect(pos, size * TILE_SIZE);
		if (SDL_RenderDrawRect(renderer_, &destrect) < 0)
			warn << "RenderDrawRect failed: " << SDL_GetError();
	}

	Vec2 cam_;
	float zoom_ = 1;
	SDL_Window *window_;
	SDL_Renderer *renderer_;
	float scale_;
	SDL_RendererInfo rinfo_;
};

}
