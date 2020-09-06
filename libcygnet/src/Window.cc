#include "Window.h"

#include <SDL_opengles2.h>

#include "glutil.h"

namespace Cygnet {

Window::Window(const char *name, int width, int height) {
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);

	win_.reset(SDL_CreateWindow(
		name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE |
		SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL));

	sdlAssert(glctx_ = SDL_GL_CreateContext(win_.get()));
	makeCurrent();
	glCheck();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glCheck();
}

Window::~Window() {
	SDL_GL_DeleteContext(glctx_);
}

void Window::makeCurrent() {
	SDL_GL_MakeCurrent(win_.get(), glctx_);
}

void Window::clear() {
	//glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Window::flip() {
	SDL_GL_SwapWindow(win_.get());
}

}
