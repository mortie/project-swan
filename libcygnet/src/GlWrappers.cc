#include "GlWrappers.h"

#include <iostream>

#include "gl.h"
#include "util.h"
#include <swan/constants.h>

namespace Cygnet {

GlShader::GlShader(Type type, const char *source)
{
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

	std::string chunkWidth = std::to_string(Swan::CHUNK_WIDTH);
	std::string chunkHeight = std::to_string(Swan::CHUNK_HEIGHT);
	std::string fluidChunkWidth = std::to_string(
		Swan::CHUNK_WIDTH * Swan::FLUID_RESOLUTION);
	std::string fluidChunkHeight = std::to_string(
		Swan::CHUNK_HEIGHT * Swan::FLUID_RESOLUTION);
	std::string tileSize = std::to_string(Swan::TILE_SIZE);
	const char *sources[] = {
		GLSL_PRELUDE,
		"\n",
		"#define SWAN_CHUNK_WIDTH ", chunkWidth.c_str(), "\n",
		"#define SWAN_CHUNK_HEIGHT ", chunkHeight.c_str(), "\n",
		"#define SWAN_CHUNK_FLUID_WIDTH ", chunkWidth.c_str(), "\n",
		"#define SWAN_CHUNK_FLUID_HEIGHT ", chunkHeight.c_str(), "\n",
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

GlShader::~GlShader()
{
	glDeleteShader(id_);
	glCheck();
}

GlProgram::GlProgram(const char *name, const char *vertex, const char *fragment): GlProgram()
{
	std::cerr << "Cygnet: Compiling " << name << "...\n";
	addShader(GlVxShader(vertex));
	addShader(GlFrShader(fragment));
	link();
}

GlProgram::GlProgram()
{
	id_ = glCreateProgram();
	glCheck();
}

GlProgram::~GlProgram()
{
	glDeleteProgram(id_);
	glCheck();
}

void GlProgram::addShader(const GlShader &shader)
{
	glAttachShader(id_, shader.id());
	glCheck();
}

void GlProgram::link()
{
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

GlFramebuffer::GlFramebuffer(GLint internalFormat, GLenum format, GLenum type):
	internalFormat_(internalFormat),
	format_(format),
	type_(type)
{}

GlFramebuffer::GlFramebuffer(GlFramebuffer &&other):
	internalFormat_(other.internalFormat_),
	format_(other.format_),
	type_(other.type_),
	fbo_(other.fbo_),
	tex_(other.tex_),
	stencilTex_(other.stencilTex_),
	withStencil_(other.withStencil_),
	size_(other.size_)
{
	other.fbo_ = 0;
	other.tex_ = 0;
	other.stencilTex_ = 0;
}

GlFramebuffer &GlFramebuffer::operator=(GlFramebuffer &&other)
{
	if (&other == this) {
		return *this;
	}

	this->~GlFramebuffer();
	new (this) GlFramebuffer(std::move(other));
	return *this;
}

void GlFramebuffer::setSize(Swan::Vec2i size)
{
	if (size == size_) {
		return;
	}

	destroy();

	// Generate texture
	glGenTextures(1, &tex_);
	glBindTexture(GL_TEXTURE_2D, tex_);
	glCheck();
	glTexImage2D(
		GL_TEXTURE_2D, 0, internalFormat_, size.x, size.y, 0,
		format_, type_, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glCheck();

	// Generate stencil texture
	if (withStencil_) {
		glGenTextures(1, &stencilTex_);
		glBindTexture(GL_TEXTURE_2D, stencilTex_);
		glCheck();
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, size.x, size.y, 0,
			GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glCheck();
	}

	// Remember previous FBO
	GLint prevFBO;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);

	// Generate framebuffer
	glGenFramebuffers(1, &fbo_);
	glCheck();
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
	glCheck();
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		tex_, 0);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
		stencilTex_, 0);
	glCheck();

	// Restore previous FBO
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
}

void GlFramebuffer::destroy()
{
	if (fbo_ != 0) {
		glDeleteFramebuffers(1, &fbo_);
		glCheck();
		fbo_ = 0;
	}

	if (tex_ != 0) {
		glDeleteTextures(1, &tex_);
		glCheck();
		tex_ = 0;
	}

	if (stencilTex_ != 0) {
		glDeleteTextures(1, &stencilTex_);
		glCheck();
		stencilTex_ = 0;
	}
}

}
