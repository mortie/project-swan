#pragma once

#include <cstdlib>
#include <string>
#include <vector>

namespace Swan {

enum class ActionKind {
	CONTINUOUS,
	ONESHOT,
	AXIS,
};

struct ActionSpec {
	ActionKind kind;
	std::string name;
	std::vector<std::string> defaultInputs;
};

struct Action {
	float activation = 0;

	operator bool() const { return std::abs(activation) > 0.4; }
};

}
