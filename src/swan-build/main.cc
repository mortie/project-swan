#include "build.h"

#include <iostream>

int main(int argc, char **argv)
{
	if (argc != 3) {
		std::cout << "Usage: " << argv[0] << " <mod> <swan>\n";
		return 1;
	}

	return SwanBuild::build(argv[1], argv[2]) ? 0 : 1;
}
