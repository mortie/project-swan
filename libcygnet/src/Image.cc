#include "Image.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdexcept>

#include "util.h"

namespace Cygnet {

// TODO: Maybe this should be conditional based on endianness?
static SDL_PixelFormatEnum format = SDL_PIXELFORMAT_ABGR8888;

Image::Image(std::string path) {
	CPtr<SDL_Surface, SDL_FreeSurface> surface(IMG_Load(path.c_str()));
	if (!surface.get()) {
		throw std::runtime_error("Failed to load " + path + ": " + IMG_GetError());
	}

	if (surface->format->format != format) {
		printf("Converting pixel format for %s", path.c_str());
		surface.reset(SDL_ConvertSurfaceFormat(surface.get(), format, 0));
		if (!surface.get()) {
			throw std::runtime_error("Failed convert " + path + ": " + SDL_GetError());
		}
	}

	w_ = surface->w;
	h_ = surface->h;
	pitch_ = surface->pitch;
	bytes_.reset(surface->pixels);
	surface->pixels = nullptr;
}

GlTexture &Image::texture() {
	if (tex_dirty_) {
		tex_.upload(w_, h_, bytes_.get(), GL_RGBA);
	}

	return tex_;
}

}
