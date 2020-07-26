#pragma once

#include <SDL.h>
#include <stdint.h>
#include <glm/mat4x4.hpp>

#include "util.h"

namespace Cygnet {

struct GlTexture;

class Window {
public:
	Window(const char *name, int width, int height);
	~Window();

	void makeCurrent();
	void clear();
	void flip();

private:
	CPtr<SDL_Window, SDL_DestroyWindow> win_;
	SDL_GLContext glctx_;
};

}
