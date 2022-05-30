#pragma once

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
#else
#include <GLES3/gl3.h>
#endif

namespace Cygnet {
extern const char *GLSL_PRELUDE;
}
