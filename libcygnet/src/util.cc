#include "util.h"

#include <stdlib.h>
#include <iostream>

#include "gl.h"

namespace Cygnet {

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

void glCheck() {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		static bool throwError = [] {
			char *str = getenv("SWAN_IGNORE_GL_ERROR");
			return str == nullptr || str[0] != '1' || str[1] != '\0';
		}();

		if (throwError) {
			throw GlError(glErrorString(err));
		} else {
			std::cerr << "GL error: " << glErrorString(err) << '\n';
		}
	}
}

}
