#include "OS.h"

#include <stdexcept>

#if defined(__MINGW32__)
#else
#include <unistd.h>
#include <dlfcn.h>
#endif

namespace Swan {

namespace OS {

#if defined(__MINGW32__)
Dynlib::Dynlib(const std::string &path)
{
	// TODO
	handle_ = nullptr;
}

Dynlib::~Dynlib()
{
	// TODO
}

void *Dynlib::getVoid(const std::string &name)
{
	// TODO
	return nullptr;
}
#else
Dynlib::Dynlib(const std::string &path)
{
	handle_ = dlopen(cat(path, ".so").c_str(), RTLD_LAZY);
	if (!handle_) {
		throw std::runtime_error(dlerror());
	}
}

Dynlib::~Dynlib()
{
	if (handle_) {
		dlclose(handle_);
	}
}

void *Dynlib::getVoid(const std::string &name)
{
	return dlsym(handle_, name.c_str());
}
#endif

Dynlib::Dynlib(Dynlib &&dl) noexcept: handle_(dl.handle_)
{
	dl.handle_ = nullptr;
}

Dynlib &Dynlib::operator=(Dynlib &&dl) noexcept
{
	handle_ = dl.handle_;
	dl.handle_ = nullptr;
	return *this;
}

}

}
