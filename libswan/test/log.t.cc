#include "log.h"

#include <sstream>

#include "lib/test.h"

test("Basic logging") {
	std::stringstream ostream;
	Swan::Logger log(ostream, "test");
	log << "Hello World, a number: " << 100 << ", and a string";
	expecteq(ostream.str(), "test: Hello World, a number: 100, and a string\n");
}
