#pragma once

#include <initializer_list>
#include <utility>
#include <cygnet/util.h>

namespace Swan {

namespace Draw {

Cygnet::Color linearGradient(
	float val, std::initializer_list<std::pair<float, Cygnet::Color> > colors);

}

}
