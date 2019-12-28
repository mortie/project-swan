#pragma once

#include <iostream>

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

	Logger(std::ostream &os, std::string name): os_(os), name_(std::move(name)) {}

	template<typename T>
	NewlineStream operator<<(const T &val) {
		os_ << name_ << ": " << val;
		return NewlineStream(os_);
	}

private:
	std::ostream &os_;
	std::string name_;
};

static Logger log(std::clog, "log");
static Logger info(std::clog, "info");
static Logger warning(std::clog, "warning");
static Logger panic(std::clog, "panic");

}
