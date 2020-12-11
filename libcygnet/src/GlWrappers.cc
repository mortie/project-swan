#include "GlWrappers.h"

#include <SDL_opengles2.h>
#include <iostream>

#include "util.h"

namespace Cygnet {

GlShader::GlShader(Type type, const char *source) {
	switch (type) {
	case Type::VERTEX:
		id_ = glCreateShader(GL_VERTEX_SHADER);
		glCheck();
		std::cerr << "Cygnet: Compiling vertex shader...\n";
		break;

	case Type::FRAGMENT:
		id_ = glCreateShader(GL_FRAGMENT_SHADER);
		glCheck();
		std::cerr << "Cygnet: Compiling fragment shader...\n";
		break;
	}

	glShaderSource(id_, 1, &source, NULL);
	glCheck();
	glCompileShader(id_);
	glCheck();

	char log[4096];
	GLsizei length = 0;
	glGetShaderInfoLog(id_, sizeof(log), &length, log);
	glCheck();
	if (length != 0) {
		std::cerr << "Cygnet: Shader compile info (" << length << "):\n" << log << '\n';
	}

	GLint status = 0;
	glGetShaderiv(id_, GL_COMPILE_STATUS, &status);
	glCheck();
	if (status == GL_FALSE) {
		throw GlCompileError("GL shader compilation failed.");
	}
}

GlShader::~GlShader() {
	glDeleteShader(id_);
	glCheck();
}

GlProgram::GlProgram() {
	id_ = glCreateProgram();
	glCheck();
}

GlProgram::~GlProgram() {
	glDeleteProgram(id_);
	glCheck();
}

void GlProgram::addShader(const GlShader &shader) {
	glAttachShader(id_, shader.id());
	glCheck();
}

GLint GlProgram::attribLoc(const char *name) {
	return glGetAttribLocation(id_, name);
}

GLint GlProgram::uniformLoc(const char *name) {
	return glGetUniformLocation(id_, name);
}

void GlProgram::link() {
	std::cout << "Cygnet: Linking...\n";
	glLinkProgram(id_);
	glCheck();

	char log[4096];
	GLsizei length = 0;
	glGetProgramInfoLog(id_, sizeof(log), &length, log);
	glCheck();
	if (length != 0) {
		std::cerr << "Cygnet: Program link info:\n" << log << '\n';
	}

	GLint status = 0;
	glGetProgramiv(id_, GL_LINK_STATUS, &status);
	glCheck();
	if (status == GL_FALSE) {
		throw GlCompileError("GL program link failed.");
	}
}

GlTexture::GlTexture() {
	glGenTextures(1, &id_);
	glCheck();
}

void GlTexture::upload(GLsizei width, GLsizei height, void *data,
		GLenum format, GLenum type) {
	w_ = width;
	h_ = height;
	glBindTexture(GL_TEXTURE_2D, id_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, type, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glCheck();
}

}
