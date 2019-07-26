#include "Timer.h"

#include <time.h>

namespace Swan {

Timer &Timer::start() {
	start_ = now();
	return *this;
}

Timer &Timer::print(const std::string &str) {
	double t = now() - start_;
	if (t > 1)
		fprintf(stderr, "%s: %.2fs\n", str.c_str(), t);
	else if (t > 0.001)
		fprintf(stderr, "%s: %.2fms\n", str.c_str(), t * 1000);
	else
		fprintf(stderr, "%s: %.2fμ\n", str.c_str(), t * 1000000);

	return *this;
}

double Timer::now() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (double)ts.tv_sec + ((double)ts.tv_nsec / 1000000000.0);
}

}
