#include "glutil.h"

#include <SDL.h>
#include <SDL_opengles2.h>
#include <stdexcept>

namespace Cygnet {

const char *glErrorString(int err) {
#define errcase(x) case x: return #x
	switch (err) {
	errcase(GL_NO_ERROR);
	errcase(GL_INVALID_ENUM);
	errcase(GL_INVALID_VALUE);
	errcase(GL_INVALID_OPERATION);
	errcase(GL_INVALID_FRAMEBUFFER_OPERATION);
	errcase(GL_OUT_OF_MEMORY);
	default: return "(unknown)";
	}
#undef errcase
}

void glCheck() {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		throw std::runtime_error(glErrorString(err));
	}
}

void sdlAssert(bool val) {
	if (!val) {
		throw std::runtime_error(SDL_GetError());
	}
}

}
