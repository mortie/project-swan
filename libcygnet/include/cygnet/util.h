#pragma once

#include <stdexcept>
#include <stdint.h>
#include <SDL.h>

namespace Cygnet {

// I don't want every use of Cygnet to drag in OpenGL headers.
using GLint = int32_t;
using GLuint = uint32_t;
using GLsizei = int32_t;
using GLenum = uint32_t;

struct SDLError: public std::exception {
	SDLError(std::string msg): message(std::move(msg)) {}
	const char *what() const noexcept override { return message.c_str(); }
	std::string message;
};

struct GlError: public std::exception {
	GlError(std::string msg): message(std::move(msg)) {}
	const char *what() const noexcept override { return message.c_str(); }
	std::string message;
};

void sdlCheck(bool ok);
void glCheck();

}
