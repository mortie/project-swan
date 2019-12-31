#pragma once

#include <string>
#include <ostream>
#include <stdio.h>

namespace Swan {
namespace OS {

bool isTTY(FILE *f);

class Dynlib {
public:
	Dynlib(const std::string &path);
	Dynlib(const Dynlib &) = delete;
	Dynlib(Dynlib &&dl) noexcept;
	~Dynlib();

	template<typename T> T get(const std::string &name) { return (T)getVoid(name); }
	void *getVoid(const std::string &name);

private:
	void *handle_;
};

}
}
