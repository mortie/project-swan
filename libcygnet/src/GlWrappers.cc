#include "GlWrappers.h"

#include <iostream>
#include <stdexcept>

#include "glutil.h"

namespace Cygnet {

GlShader::GlShader(Type type, const char *source) {
	switch (type) {
	case Type::VERTEX:
		id_ = glCreateShader(GL_VERTEX_SHADER);
		glCheck();
		std::cerr << "compiling vertex shader...\n";
		break;
	case Type::FRAGMENT:
		id_ = glCreateShader(GL_FRAGMENT_SHADER);
		glCheck();
		std::cerr << "compiling fragment shader...\n";
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
		std::cerr << "Shader compile info (" << length << "):\n" << log << '\n';
	}

	GLint status = 0;
	glGetShaderiv(id_, GL_COMPILE_STATUS, &status);
	glCheck();
	if (status == GL_FALSE) {
		throw std::runtime_error("GL shader compilation failed.");
	}
}

GlShader::~GlShader() {
	glDeleteShader(id_);
	glCheck();
}

void GlProgram::link() {
	std::cout << "link\n";
	glLinkProgram(id_);
	glCheck();

	char log[4096];
	GLsizei length = 0;
	glGetProgramInfoLog(id_, sizeof(log), &length, log);
	glCheck();
	if (length != 0) {
		std::cerr << "Program link info:\n" << log << '\n';
	}

	GLint status = 0;
	glGetProgramiv(id_, GL_LINK_STATUS, &status);
	glCheck();
	if (status == GL_FALSE) {
		throw std::runtime_error("GL program link failed.");
	}
}

GlProgram::~GlProgram() {
	glDeleteProgram(id_);
}

}