#include "Timer.h"

#include <time.h>

namespace Swan {

bool Timer::periodic(float secs) {
	if (time_ >= secs) {
		time_ = 0;
		return true;
	}

	return false;
}

}
