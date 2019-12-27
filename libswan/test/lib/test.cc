#include "test.h"

#include <stdlib.h>
#include <string_view>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>

namespace testlib {

static const std::string color_reset = "\033[0m";
static const std::string color_highlight = "\033[1m";
static const std::string color_testing = color_highlight;
static const std::string color_desc = "\033[33m";
static const std::string color_maybe = "\033[35m";
static const std::string color_success = "\033[32m";
static const std::string color_fail = "\033[31m";
static const std::string color_errormsg = "\033[95m";

std::string color(const std::string &color, std::string_view str) {
	return std::string(color) + std::string(str) + std::string(color_reset);
}

struct TestCase {
	TestCase(TestSpec *spec):
		func(spec->func), description(spec->description),
		filename(spec->filename), linenum(spec->linenum), index(spec->index) {}

	void (*func)();
	std::string_view description;
	std::string_view filename;
	int linenum;
	int index;
};

static std::vector<TestCase> cases;

void addTestCase(TestSpec *testcase) {
	cases.emplace_back(testcase);
}

static std::stringstream printFailure(const std::string &msg) {
	std::stringstream str;
	str
		<< "\r" << color(color_highlight + color_fail, "✕ ")
		<< color(color_fail, "Failed:  ") << "\n"
		<< "    " << msg << "\n";
	return str;
}

static std::stringstream printFailure(const TestFailure &failure) {
	std::stringstream str;
	str
		<< printFailure(failure.message).str()
		<< "    at " << failure.filename << ":" << failure.linenum << "\n";
	return str;
}

}

int main() {
	using namespace testlib;

	std::sort(begin(cases), end(cases), [](TestCase &a, TestCase &b) {
		if (a.filename != b.filename)
			return a.filename < b.filename;
		return a.index < b.index;
	});

	std::string_view currfile = "";
	bool failed = false;
	for (TestCase &testcase: cases) {
		if (currfile != testcase.filename) {
			currfile = testcase.filename;
			size_t lastslash = currfile.find_last_of('/');
			std::cout << '\n' << color(color_testing, currfile.substr(lastslash + 1)) << ":\n";
		}

		std::cout
			<< color(color_highlight + color_maybe, "? ")
			<< color(color_maybe, "Testing: ")
			<< color(color_desc, testcase.description) << " " << std::flush;

		try {
			testcase.func();
			std::cout
				<< "\r" << color(color_highlight + color_success, "✓ ")
				<< color(color_success, "Success: ") << "\n";
		} catch (const TestFailure &failure) {
			failed = true;
			std::cout << printFailure(failure).str();
		} catch (const std::exception &ex) {
			failed = true;
			std::cout << printFailure(ex.what()).str();
		} catch (const std::string &str) {
			failed = true;
			std::cout << printFailure(str).str();
		} catch (const char *str) {
			failed = true;
			std::cout << printFailure(str).str();
		} catch (...) {
			failed = true;
			std::cout << printFailure("Unknown error.").str();
		}
	}

	std::cout << '\n';

	if (failed)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
