#include <SDL2/SDL.h>
#include <ostream>

#include "util.h"

namespace Swan {

inline std::ostream &operator<<(std::ostream &os, const SDL_Rect &rect) {
	os
		<< "SDL_Rect(" << rect.x << ", " << rect.y << ", "
		<< rect.w << ", " << rect.h << ")";
	return os;
}

class TexLock: NonCopyable {
public:
	TexLock(SDL_Texture *tex, SDL_Rect *rect = nullptr);
	~TexLock() { SDL_UnlockTexture(tex_); }

	int blit(SDL_Rect *destrect, SDL_Surface *srcsurf, SDL_Rect *srcrect = nullptr) {
		return SDL_BlitSurface(srcsurf, srcrect, surf_.get(), destrect);
	}

private:
	SDL_Texture *tex_;
	RaiiPtr<SDL_Surface> surf_ = makeRaiiPtr<SDL_Surface>(nullptr, SDL_FreeSurface);
};

class TexColorMod: NonCopyable {
public:
	TexColorMod(SDL_Texture *tex, uint8_t r, uint8_t g, uint8_t b): tex_(tex) {
		SDL_GetTextureColorMod(tex_, &r_, &g_, &b_);
		SDL_SetTextureColorMod(tex_, r, g, b);
	}

	~TexColorMod() {
		SDL_SetTextureColorMod(tex_, r_, g_, b_);
	}

private:
	SDL_Texture *tex_;
	uint8_t r_, g_, b_;
};

class TexAlphaMod: NonCopyable {
public:
	TexAlphaMod(SDL_Texture *tex, uint8_t alpha): tex_(tex) {
		SDL_GetTextureAlphaMod(tex_, &alpha_);
		SDL_SetTextureAlphaMod(tex_, alpha);
	}

	~TexAlphaMod() {
		SDL_SetTextureAlphaMod(tex_, alpha_);
	}

private:
	SDL_Texture *tex_;
	uint8_t alpha_;
};

}
