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

	GlShader(Type type, const char *source);
	~GlShader();

	GLuint id() const { return id_; }

private:
	GLuint id_;
};

class GlProgram: NonCopyable {
public:
	template <typename... T, typename = std::enable_if_t<std::conjunction_v<std::is_same<T, GlShader>...>>>
	GlProgram(const T &... shaders): GlProgram() { (addShader(shaders), ...); link(); }
	GlProgram() { id_ = glCreateProgram(); }
	~GlProgram();

	void addShader(const GlShader &shader) { glAttachShader(id_, shader.id()); }
	void link();

	void use() { glUseProgram(id_); }
	GLuint id() { return id_; }
	GLint attribLocation(const char *name) { return glGetAttribLocation(id_, name); }
	GLint uniformLocation(const char *name) { return glGetAttribLocation(id_, name); }

private:
	GLuint id_;
};

}