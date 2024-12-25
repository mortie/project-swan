#pragma once

#include "cygnet/util.h"
#include <stdint.h>

namespace Swan {

struct Fluid {
	using ID = uint8_t;

	struct Builder {
		std::string name;
		Cygnet::Color color;
		float density = 1;
	};

	ID id;
	std::string name;
	Cygnet::Color color;
	float density;

	Fluid() = default;
	Fluid(ID id, std::string name, const Builder &builder):
		id(id), name(name), color(builder.color), density(builder.density) {}
};

}
