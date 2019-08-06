#pragma once

#include <string>

namespace Swan {

class Timer {
public:
	void tick(float dt) { time_ += dt; }
	void reset() { time_ = 0; }
	bool periodic(float secs);

private:
	float time_ = 0;
};

}
