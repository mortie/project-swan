#pragma once

#include "swan/log.h"
#include <chrono>
#include <ostream>
#include <string_view>

namespace Swan {

class Clock {
public:
	void tick(float dt)
	{
		time_ += dt;
	}

	void reset()
	{
		time_ = 0;
	}

	float duration()
	{
		return time_;
	}

	bool periodic(float secs);

private:
	float time_ = 0;
};

class RTClock {
public:
	void reset()
	{
		start_ = std::chrono::steady_clock::now();
	}

	double duration() const
	{
		return std::chrono::duration<double>(
			std::chrono::steady_clock::now() - start_).count();
	}

	friend std::ostream &operator<<(std::ostream &os, const RTClock &clock)
	{
		double dur = clock.duration();

		if (dur > 1) {
			os << dur << 's';
		}
		else{
			os << dur * 1000.0 << "ms";
		}
		return os;
	}

private:
	std::chrono::time_point<std::chrono::steady_clock> start_ =
		std::chrono::steady_clock::now();
};

class RTDeadline: public RTClock {
public:
	RTDeadline(double deadline): deadline_(deadline) {};

	bool passed()
	{
		return duration() >= deadline_;
	}

private:
	double deadline_;
};

class ScopedTimer {
public:
	ScopedTimer(std::string_view name): name_(name) {}
	~ScopedTimer()
	{
		info << "Timer '" << name_ << "': " << clock_;
	}

private:
	RTClock clock_;
	std::string_view name_;
};

}
