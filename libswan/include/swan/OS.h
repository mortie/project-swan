#pragma once

#include <string>

namespace Swan {
namespace OS {

class Dynlib {
public:
	Dynlib(const std::string &path);
	~Dynlib();

	template<typename T> T get(const std::string &name) { return (T)getVoid(name); }
	void *getVoid(const std::string &name);

private:
	void *handle_;
};

}
}
