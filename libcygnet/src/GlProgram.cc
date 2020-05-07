#include "GlProgram.h"

#include <iostream>
#include <stdexcept>

namespace Cygnet {

GlShader::GlShader(const char *source, Type type) {
	switch (type) {
	case Type::VERTEX:
		id_ = glCreateShader(GL_VERTEX_SHADER);
		break;
	case Type::FRAGMENT:
		id_ = glCreateShader(GL_FRAGMENT_SHADER);
		break;
	}

	glShaderSource(id_, 1, &source, NULL);
	glCompileShader(id_);

	char log[4096];
	GLsizei length;
	glGetShaderInfoLog(id_, sizeof(log), &length, log);
	if (length != 0) {
		std::cerr << "Shader compile info:\n" << log << '\n';
	}

	GLint status;
	glGetShaderiv(id_, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		id_ = -1;
		throw std::runtime_error("GL shader compilation failed.");
	}

	valid_ = true;
}

GlShader::~GlShader() {
	if (valid_) {
		glDeleteShader(id_);
	}
}

void GlProgram::link() {
	std::cout << "link\n";
	glLinkProgram(id_);

	char log[4096];
	GLsizei length;
	glGetProgramInfoLog(id_, sizeof(log), &length, log);
	if (length != 0) {
		std::cerr << "Program link info:\n" << log << '\n';
	}

	GLint status;
	glGetProgramiv(id_, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		id_ = -1;
		throw std::runtime_error("GL program link failed.");
	}

	valid_ = true;
}

GlProgram::~GlProgram() {
	if (valid_) {
		glDeleteProgram(id_);
	}
}

}
