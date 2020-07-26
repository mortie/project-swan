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
	GLint attribLoc(const char *name);
	GLint attribLoc(const char *name, GLuint index);
	GLint uniformLoc(const char *name);

private:
	GLuint id_;
};

class GlTexture: NonCopyable {
public:
	GlTexture();

	void bind();
	void upload(GLsizei width, GLsizei height, void *data,
			GLenum format, GLenum type = GL_UNSIGNED_BYTE);
	GLuint id() { return id_; }

private:
	GLuint id_;
};

inline GLint GlProgram::attribLoc(const char *name) {
	return glGetAttribLocation(id_, name);
}

inline GLint GlProgram::attribLoc(const char *name, GLuint index) {
	glBindAttribLocation(id_, index, name);
	return index;
}

inline GLint GlProgram::uniformLoc(const char *name) {
	return glGetUniformLocation(id_, name);
}

}
