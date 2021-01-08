#include "test.h"

#include <stdlib.h>
#include <string_view>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>

namespace testlib {

static const std::string COLOR_RESET = "\033[0m";
static const std::string COLOR_HIGHLIGHT = "\033[1m";
static const std::string COLOR_TESTING = COLOR_HIGHLIGHT;
static const std::string COLOR_DESC = "\033[33m";
static const std::string COLOR_MAYBE = "\033[35m";
static const std::string COLOR_SUCCESS = "\033[32m";
static const std::string COLOR_FAIL = "\033[31m";
static const std::string COLOR_ERRORMSG = "\033[95m";

std::string color(const std::string &color, std::string_view str) {
	return std::string(color) + std::string(str) + std::string(COLOR_RESET);
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

struct TestFile {
	std::string_view filename;
	std::string prettyname;
	std::vector<TestCase> cases;
};

// This avoids initialization order dependencies;
// test specs' constructors will call addTestCase(),
// addTestCase will use the vector of test cases,
// and if this was just a static global vector,
// it might not have been initialized yet.
static auto &cases() {
	static std::vector<TestCase> cases;
	return cases;
}

void addTestCase(TestSpec *testcase) {
	cases().emplace_back(testcase);
}

static std::stringstream printFailure(const std::string &msg) {
	std::stringstream str;
	str
		<< "\r" << color(COLOR_HIGHLIGHT + COLOR_FAIL, "✕ ")
		<< color(COLOR_FAIL, "Failed:  ") << "\n"
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

	std::sort(begin(cases()), end(cases()), [](TestCase &a, TestCase &b) {
		if (a.filename != b.filename)
			return a.filename < b.filename;
		return a.index < b.index;
	});

	int totaltests = 0;
	int totalsuccess = 0;

	std::string_view currfile = "";
	bool failed = false;
	for (TestCase &testcase: cases()) {
		if (currfile != testcase.filename) {
			currfile = testcase.filename;
			size_t lastslash = currfile.find_last_of('/');
			std::cout << '\n' << color(COLOR_TESTING, currfile.substr(lastslash + 1)) << ":\n";
		}

		std::cout
			<< color(COLOR_HIGHLIGHT + COLOR_MAYBE, "? ")
			<< color(COLOR_MAYBE, "Testing: ")
			<< color(COLOR_DESC, testcase.description) << " " << std::flush;

		bool casefailed = false;

		try {
			totaltests += 1;
			testcase.func();
			std::cout
				<< "\r" << color(COLOR_HIGHLIGHT + COLOR_SUCCESS, "✓ ")
				<< color(COLOR_SUCCESS, "Success: ")
				<< color(COLOR_DESC, testcase.description) << "\n";
			totalsuccess += 1;
		} catch (const TestFailure &failure) {
			casefailed = true;
			std::cout << printFailure(failure).str();
		} catch (const std::exception &ex) {
			failed = true;
			std::cout << printFailure(ex.what()).str();
		} catch (const std::string &str) {
			casefailed = true;
			std::cout << printFailure(str).str();
		} catch (const char *str) {
			casefailed = true;
			std::cout << printFailure(str).str();
		} catch (...) {
			casefailed = true;
			std::cout << printFailure("Unknown error.").str();
		}

		if (casefailed)
			failed = true;
	}

	std::cout << '\n';

	std::cout << "Total: " << totalsuccess << '/' << totaltests << "\n\n";

	if (failed)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
