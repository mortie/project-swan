#include "GlWrappers.h"

#include <iostream>

#include "gl.h"
#include "util.h"
#include <swan-common/constants.h>

namespace Cygnet {

GlShader::GlShader(Type type, const char *source) {
	const char *t = nullptr;
	switch (type) {
	case Type::VERTEX:
		id_ = glCreateShader(GL_VERTEX_SHADER);
		glCheck();
		t = "vertex";
		break;

	case Type::FRAGMENT:
		id_ = glCreateShader(GL_FRAGMENT_SHADER);
		glCheck();
		t = "fragment";
		break;
	}

	std::string chunkWidth = std::to_string(SwanCommon::CHUNK_WIDTH);
	std::string chunkHeight = std::to_string(SwanCommon::CHUNK_HEIGHT);
	std::string tileSize = std::to_string(SwanCommon::TILE_SIZE);
	const char *sources[] = {
		GLSL_PRELUDE,
		"\n",
		"#define SWAN_CHUNK_WIDTH ", chunkWidth.c_str(), "\n",
		"#define SWAN_CHUNK_HEIGHT ", chunkHeight.c_str(), "\n",
		"#define SWAN_TILE_SIZE ", tileSize.c_str(), "\n",
		"\n",
		source,
	};
	size_t sourcecount = sizeof(sources) / sizeof(*sources);

	glShaderSource(id_, sourcecount, sources, NULL);
	glCheck();
	glCompileShader(id_);
	glCheck();

	char log[4096];
	GLsizei length = 0;
	glGetShaderInfoLog(id_, sizeof(log), &length, log);
	glCheck();
	if (length != 0) {
		std::cerr << "Cygnet: Shader compile info (" << t << "):\n" << log << '\n';
	}

	GLint status = 0;
	glGetShaderiv(id_, GL_COMPILE_STATUS, &status);
	glCheck();
	if (status == GL_FALSE) {
		std::cerr << "Cygnet: Here's the broken shader:\n";
		for (size_t i = 0; i < sourcecount; ++i) {
			std::cerr << sources[i];
		}
		std::cerr << '\n';
		throw GlCompileError("GL shader compilation failed.");
	}
}

GlShader::~GlShader() {
	glDeleteShader(id_);
	glCheck();
}

GlProgram::GlProgram(const char *name, const char *vertex, const char *fragment): GlProgram() {
	std::cerr << "Cygnet: Compiling " << name << "...\n";
	addShader(GlVxShader(vertex));
	addShader(GlFrShader(fragment));
	link();
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

void GlProgram::link() {
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

}
