#pragma once

#ifndef SWAN_HEADLESS

// IWYU pragma: begin_exports
#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
#elif defined(__MINGW32__)
#include <glad/glad.h>
#elif defined(__linux__)
#include <GLES3/gl3.h>
#else
#error "Unknown architecture"
#endif
// IWYU pragma: end_exports

#else
typedef int GLint;
typedef unsigned int GLuint;
#endif

namespace Cygnet {

extern const char *GLSL_PRELUDE;

}
