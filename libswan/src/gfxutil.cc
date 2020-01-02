#include "gfxutil.h"

#include <stdint.h>

#include "log.h"

namespace Swan {

TexLock::TexLock(SDL_Texture *tex, SDL_Rect *rect): tex_(tex) {

	// We must query the texture to get a format...
	uint32_t format;
	int access, texw, texh;
	if (SDL_QueryTexture(tex_, &format, &access, &texw, &texh) < 0) {
		panic << "Failed to query texture: " << SDL_GetError();
		abort();
	}

	SDL_Rect lockrect = rect == NULL
		? SDL_Rect{ 0, 0, texw, texh }
		: *rect;

	// ...and convert that format into masks...
	int bpp = 32;
	uint32_t rmask, gmask, bmask, amask;
	if (SDL_PixelFormatEnumToMasks(format, &bpp, &rmask, &gmask, &bmask, &amask) != SDL_TRUE) {
		panic << "Failed to get pixel mask: " << SDL_GetError();
		abort();
	}

	// ...and lock the texture...
	uint8_t *pixels;
	int pitch;
	if (SDL_LockTexture(tex_, &lockrect, (void **)&pixels, &pitch) < 0) {
		panic << "Failed to lock texture: " << SDL_GetError();
		abort();
	}

	// ...in order to create a surface.
	surf_.reset(SDL_CreateRGBSurfaceFrom(
		pixels, lockrect.w, lockrect.h,
		32, pitch, rmask, gmask, bmask, amask));
}

TexLock::TexLock(TexLock &&lock) noexcept {
	tex_ = lock.tex_;
	surf_ = std::move(lock.surf_);
	lock.tex_ = nullptr;
}

TexLock::~TexLock() {
	if (tex_)
		SDL_UnlockTexture(tex_);
}

}
