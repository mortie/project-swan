#pragma once

#include <string>
#include <stdio.h>

#include "util.h"

namespace Swan {
namespace OS {

bool isTTY(FILE *f);

class Dynlib: NonCopyable {
public:
	Dynlib(const std::string &path);
	Dynlib(Dynlib &&dl) noexcept;
	~Dynlib();

	Dynlib &operator=(Dynlib &&dl) noexcept;

	template<typename T> T get(const std::string &name) { return (T)getVoid(name); }
	void *getVoid(const std::string &name);

private:
	void *handle_;
};

}
}
