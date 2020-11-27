#pragma once

#include <stdint.h>
#include <stdexcept>

#include "util.h"

namespace Cygnet {

struct GlCompileError: public std::exception {
	GlCompileError(std::string message): message(std::move(message)) {}
	const char *what() const noexcept override { return message.c_str(); }
	std::string message;
};

class GlShader {
public:
	enum class Type {
		VERTEX,
		FRAGMENT,
	};

	GlShader(Type type, const char *source);
	~GlShader();

	GlID id() const { return id_; }

private:
	GlID id_;
};

class GlVxShader: public GlShader {
public:
	explicit GlVxShader(const char *source): GlShader(Type::VERTEX, source) {}
};

class GlFrShader: public GlShader {
public:
	explicit GlFrShader(const char *source): GlShader(Type::FRAGMENT, source) {}
};

class GlProgram {
public:
	template <typename... T>
	GlProgram(const T &... shaders): GlProgram() { (addShader(shaders), ...); link(); }
	GlProgram();
	~GlProgram();

	void use();
	GlID id() const { return id_; }

protected:

	GlLoc attribLoc(const char *name);
	GlLoc uniformLoc(const char *name);

private:
	void addShader(const GlShader &shader);
	void link();

	GlID id_;
};

}
