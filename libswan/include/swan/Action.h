#pragma once

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

class Action {
public:
	Action(): activation_(&DUMMY) {}
	Action(float *activation): activation_(activation) {}

	operator bool() const { return direction() != 0; }
	float value() const { return *activation_; }

	int direction() const
	{
		if (*activation_ < -0.4) {
			return -1;
		} else if (*activation_ > 0.4) {
			return 1;
		} else {
			return 0;
		}
	}

private:
	float *activation_ = 0;
	inline static float DUMMY = 0;
};

}
