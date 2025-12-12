#include "OS.h"

#include <stdexcept>

#if defined(__MINGW32__)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#else
#include <unistd.h>
#include <dlfcn.h>
#endif

namespace Swan {

namespace OS {

#if defined(__MINGW32__)
Dynlib::Dynlib(const std::string &path)
{
	HINSTANCE inst = LoadLibrary(cat(path, ".dll").c_str());
	if (!inst) {
		auto err = cat("Loading '", path, "' failed: ", GetLastError());
		throw std::runtime_error(err);
	}

	handle_ = (void *)inst;
}

Dynlib::~Dynlib()
{
	if (handle_) {
		FreeLibrary((HINSTANCE)handle_);
	}
}

void *Dynlib::getVoid(const std::string &name)
{
	FARPROC fp = GetProcAddress((HINSTANCE)handle_, name.c_str());
	return (void *)fp;
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
