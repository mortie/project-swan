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
	std::string name;
	ActionKind kind;
	std::vector<std::string> defaultInputs;
};

struct Action {
	float activation = 0;

	operator bool() const { return direction() != 0; }

	int direction() const
	{
		if (activation < -0.4) {
			return -1;
		} else if (activation > 0.4) {
			return 1;
		} else {
			return 0;
		}
	}
};

}
