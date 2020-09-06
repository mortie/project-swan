#pragma once

#include <SDL.h>
#include <stdint.h>

#include "util.h"

namespace Cygnet {

class GlTexture;

class Window {
public:
	Window(const char *name, int width, int height);
	~Window();

	void makeCurrent();
	void clear();
	void flip();
	int width() { return w_; }
	int height() { return h_; }

	// xScale and yScale are what drawn objects need to be scaled with
	// in order to have square pixels.
	float xScale() { return xScale_; }
	float yScale() { return yScale_; }

	void onResize(int w, int h);

private:
	CPtr<SDL_Window, SDL_DestroyWindow> win_;
	SDL_GLContext glctx_;
	int w_;
	int h_;
	float yScale_;
	float xScale_;
};

}
