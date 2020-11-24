#pragma once

#include <stdexcept>
#include <SDL_opengles2.h>
#include <SDL.h>

namespace Cygnet {

struct SDLError: public std::exception {
	SDLError(std::string msg): message(std::move(msg)) {}
	std::string message;

	const char *what() const noexcept override {
		return message.c_str();
	}
};

struct GLError: public std::exception {
	GLError(std::string msg): message(std::move(msg)) {}
	std::string message;

	const char *what() const noexcept override {
		return message.c_str();
	}
};

inline const char *glErrorString(int err) {
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

inline void sdlCheck(bool ok) {
	if (!ok) {
		throw SDLError(SDL_GetError());
	}
}

inline void glCheck() {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		throw GLError(glErrorString(err));
	}
}

}
