#include "OS.h"

#include <stdexcept>
#include <unistd.h>
#include <dlfcn.h>

namespace Swan {
namespace OS {

Dynlib::Dynlib(const std::string &path) {
	handle_ = dlopen((path + ".so").c_str(), RTLD_LAZY);
	if (handle_ == NULL) {
		throw std::runtime_error(dlerror());
	}
}

Dynlib::~Dynlib() {
	dlclose(handle_);
}

void *Dynlib::getVoid(const std::string &name) {
	return dlsym(handle_, name.c_str());
}

}
}
