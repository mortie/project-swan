#pragma once

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/OpenGL.h> // IWYU pragma: export
#include <OpenGL/gl3.h> // IWYU pragma: export
#else
#include <GLES3/gl3.h> // IWYU pragma: export
#endif

namespace Cygnet {
extern const char *GLSL_PRELUDE;
}
