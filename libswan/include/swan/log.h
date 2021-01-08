#pragma once

#include <iostream>

#include "OS.h"

namespace Swan {

class Logger {
public:
	class NewlineStream {
	public:
		NewlineStream(std::ostream &os): os_(os) {}
		~NewlineStream() {
			os_ << '\n' << std::flush;
		}

		template<typename T>
		NewlineStream &operator<<(const T &val) {
			os_ << val;
			return *this;
		}

	private:
		std::ostream &os_;
	};

	Logger(std::ostream &os, std::string name, bool use_color = false, std::string color = ""):
		os_(os), name_(std::move(name)), useColor_(use_color), color_(std::move(color)) {}

	template<typename T>
	NewlineStream operator<<(const T &val) {
		if (useColor_)
			os_ << color_ << name_ << "\033[0m: " << val;
		else
			os_ << name_ << ": " << val;
		return NewlineStream(os_);
	}

private:
	std::ostream &os_;
	std::string name_;
	bool useColor_;
	std::string color_;
};

static std::ostream &logstream = std::clog;
static Logger info(logstream, "info", OS::isTTY(stderr), "\033[36m");
static Logger warn(logstream, "warning", OS::isTTY(stderr), "\033[33m");
static Logger panic(logstream, "panic", OS::isTTY(stderr), "\033[1m\033[31m");

}
