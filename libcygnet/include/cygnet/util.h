#pragma once

#include <stdint.h>

#include "swan/Matrix3.h"

namespace Cygnet {

// I don't want every use of Cygnet to drag in OpenGL headers.
using GLint = int32_t;
using GLuint = uint32_t;
using GLsizei = int32_t;
using GLenum = uint32_t;
using GLfloat = float;

using Mat3gf = Swan::Matrix3<GLfloat>;

struct ByteColor;

struct Color {
	float r = 0, g = 0, b = 0, a = 1;

	static Color premultiply(float r, float g, float b, float a)
	{
		return {r * a, g * a, b * a, a};
	}

	operator ByteColor() const;
	friend bool operator==(const Color &a, const Color &b) = default;
};

struct ByteColor {
	uint8_t r = 0, g = 0, b = 0, a = 0;

	operator Color() const;
	friend bool operator==(const ByteColor &a, const ByteColor &b) = default;
};

inline Color::operator ByteColor() const
{
	return {uint8_t(r * 255), uint8_t(g * 255), uint8_t(b * 255), uint8_t(a * 255)};
}

inline ByteColor::operator Color() const
{
	return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
}

struct GlError: public std::exception {
	GlError(std::string msg): message(std::move(msg))
	{}
	const char *what() const noexcept override
	{
		return message.c_str();
	}

	std::string message;
};

void glCheck();

}
