#pragma once

#include "cygnet/util.h"
#include <stdint.h>

namespace Swan {

struct Fluid {
	using ID = uint8_t;

	struct Builder {
		std::string name;
		Cygnet::Color fg;
		Cygnet::Color bg = fg;
		float density = 1;
	};

	ID id;
	std::string name;
	Cygnet::Color fg;
	Cygnet::Color bg;
	float density;

	Fluid() = default;
	Fluid(ID id, std::string name, const Builder &builder):
		id(id), name(name), fg(builder.fg), bg(builder.bg), density(builder.density) {}
};

}
