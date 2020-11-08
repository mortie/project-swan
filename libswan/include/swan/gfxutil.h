#include <SDL2/SDL.h>
#include <ostream>

#include "util.h"

#include "log.h"

namespace Swan {

inline std::ostream &operator<<(std::ostream &os, const SDL_Rect &rect) {
	os
		<< "SDL_Rect(" << rect.x << ", " << rect.y << ", "
		<< rect.w << ", " << rect.h << ")";
	return os;
}

class RenderBlendMode: NonCopyable {
public:
	RenderBlendMode(SDL_Renderer *rnd, SDL_BlendMode mode): rnd_(rnd) {
		SDL_GetRenderDrawBlendMode(rnd_, &mode_);
		SDL_SetRenderDrawBlendMode(rnd_, mode);
	}

	~RenderBlendMode() {
		SDL_SetRenderDrawBlendMode(rnd_, mode_);
	}

private:
	SDL_Renderer *rnd_;
	SDL_BlendMode mode_;
};

class RenderDrawColor: NonCopyable {
public:
	RenderDrawColor(SDL_Renderer *rnd, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255): rnd_(rnd) {
		SDL_GetRenderDrawColor(rnd_, &r_, &g_, &b_, &a_);
		SDL_SetRenderDrawColor(rnd_, r, g, b, a);
	}

	void change(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255) {
		SDL_SetRenderDrawColor(rnd_, r, g, b, a);
	}

	~RenderDrawColor() {
		SDL_SetRenderDrawColor(rnd_, r_, g_, b_, a_);
	}

private:
	SDL_Renderer *rnd_;
	Uint8 r_, g_, b_, a_;
};

class RenderClipRect: NonCopyable {
public:
	RenderClipRect(SDL_Renderer *rnd, SDL_Rect *rect): rnd_(rnd) {
		enabled_ = SDL_RenderIsClipEnabled(rnd_);
		SDL_RenderGetClipRect(rnd_, &rect_);
		SDL_RenderSetClipRect(rnd_, rect);
	}

	~RenderClipRect() {
		if (enabled_)
			SDL_RenderSetClipRect(rnd_, &rect_);
		else
			SDL_RenderSetClipRect(rnd_, nullptr);
	}

private:
	SDL_Renderer *rnd_;
	bool enabled_;
	SDL_Rect rect_;
};

class RenderTarget: NonCopyable {
public:
	RenderTarget(SDL_Renderer *rnd, SDL_Texture *tex): rnd_(rnd) {
		prev_target_ = SDL_GetRenderTarget(rnd_);
		SDL_SetRenderTarget(rnd_, tex);
	}

	~RenderTarget() {
		SDL_SetRenderTarget(rnd_, prev_target_);
	}

private:
	SDL_Renderer *rnd_;
	SDL_Texture *prev_target_;
};

class TexLock: NonCopyable {
public:
	TexLock(SDL_Texture *tex, SDL_Rect *rect = nullptr);
	~TexLock() { SDL_UnlockTexture(tex_); }

	int blit(SDL_Rect *destrect, SDL_Surface *srcsurf, SDL_Rect *srcrect = nullptr) {
		return SDL_BlitSurface(srcsurf, srcrect, surf_.get(), destrect);
	}

private:
	SDL_Texture *tex_;
	CPtr<SDL_Surface, SDL_FreeSurface> surf_;
};

class TexColorMod: NonCopyable {
public:
	TexColorMod(SDL_Texture *tex, Uint8 r, Uint8 g, Uint8 b): tex_(tex) {
		SDL_GetTextureColorMod(tex_, &r_, &g_, &b_);
		SDL_SetTextureColorMod(tex_, r, g, b);
	}

	~TexColorMod() {
		SDL_SetTextureColorMod(tex_, r_, g_, b_);
	}

private:
	SDL_Texture *tex_;
	Uint8 r_, g_, b_;
};

class TexAlphaMod: NonCopyable {
public:
	TexAlphaMod(SDL_Texture *tex, Uint8 alpha): tex_(tex) {
		SDL_GetTextureAlphaMod(tex_, &alpha_);
		SDL_SetTextureAlphaMod(tex_, alpha);
	}

	~TexAlphaMod() {
		SDL_SetTextureAlphaMod(tex_, alpha_);
	}

private:
	SDL_Texture *tex_;
	Uint8 alpha_;
};

}
