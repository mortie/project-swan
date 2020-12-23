#include "Window.h"

#include <SDL.h>

#include "gl.h"
#include "util.h"

namespace Cygnet {

struct WindowState {
	SDL_Window *window;
	SDL_GLContext glctx;
};

Window::Window(const char *name, int w, int h):
		state_(std::make_unique<WindowState>()), w_(w), h_(h) {
	state_->window = SDL_CreateWindow(name,
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h,
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE |
			SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
	sdlCheck(state_->window != NULL);

	state_->glctx = SDL_GL_CreateContext(state_->window);
	glCheck();
	makeCurrent();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glCheck();

	SDL_GetWindowSize(state_->window, &w, &h);
	onResize(w, h);
}

Window::~Window() = default;

void Window::makeCurrent() {
	SDL_GL_MakeCurrent(state_->window, state_->glctx);
	glCheck();
}

void Window::clear() {
	glClear(GL_COLOR_BUFFER_BIT);
	glCheck();
}

void Window::flip() {
	SDL_GL_SwapWindow(state_->window);
	glCheck();
}

void Window::onResize(int w, int h) {
	w_ = w;
	h_ = h;
	glViewport(0, 0, w, h);
	glCheck();
}

}
