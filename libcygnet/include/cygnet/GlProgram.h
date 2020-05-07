#pragma once

#include <vector>
#include <initializer_list>
#include <functional>
#include <SDL_opengles2.h>

#include "util.h"

namespace Cygnet {

class GlShader: NonCopyable {
public:
	enum class Type {
		VERTEX,
		FRAGMENT,
	};

	GlShader(const char *source, Type type);
	~GlShader();

	GLuint id() { return id_; }

private:
	GLuint id_;
	bool valid_ = false;
};

class GlProgram: NonCopyable {
public:
	GlProgram(std::initializer_list<std::reference_wrapper<GlShader>> shaders);
	~GlProgram();

	void use() { glUseProgram(id_); }
	GLuint id() { return id_; }
	GLuint getLocation(const char *name) { return glGetAttribLocation(id_, name); }

private:
	GLuint id_;
	bool valid_ = false;
};

}
