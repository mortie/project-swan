#include <string>

namespace Swan {

class Timer {
public:
	Timer &start();
	Timer &print(const std::string &str);

	static double now();

private:
	std::string name_;
	double start_;
};

}
