#include "Clock.h"

namespace Swan {

bool Clock::periodic(float secs) {
	if (time_ >= secs) {
		time_ = 0;
		return true;
	}

	return false;
}

}
