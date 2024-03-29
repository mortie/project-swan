#include "OS.h"

#include <stdexcept>
#include <unistd.h>
#include <dlfcn.h>

namespace Swan {

namespace OS {

bool isTTY(FILE *f)
{
	int fd = fileno(f);

	return isatty(fd);
}

Dynlib::Dynlib(const std::string &path)
{
	handle_ = dlopen(cat(path, ".so").c_str(), RTLD_LAZY);
	if (!handle_) {
		throw std::runtime_error(dlerror());
	}
}

Dynlib::Dynlib(Dynlib &&dl) noexcept: handle_(dl.handle_)
{
	dl.handle_ = nullptr;
}

Dynlib::~Dynlib()
{
	if (handle_) {
		dlclose(handle_);
	}
}

Dynlib &Dynlib::operator=(Dynlib &&dl) noexcept
{
	handle_ = dl.handle_;
	dl.handle_ = nullptr;
	return *this;
}

void *Dynlib::getVoid(const std::string &name)
{
	return dlsym(handle_, name.c_str());
}

}

}
