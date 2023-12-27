#pragma once

namespace testlib {

struct TestFailure {
	TestFailure(const char *msg, const char *file, int line):
		message(msg), filename(file), linenum(line)
	{}

	const char *message;
	const char *filename;
	int linenum;
};

struct TestSpec;
void addTestCase(TestSpec *testcase);

struct TestSpec {
	TestSpec(void (*f)(), const char *desc, const char *file, int line, int idx):
		func(f), description(desc), filename(file), linenum(line), index(idx)
	{
		addTestCase(this);
	}

	void (*func)();
	const char *description;
	const char *filename;
	int linenum;
	int index;
};

}

#define TEST3(name, id) name ## id
#define TEST2(uniqid, desc) \
		static void TEST3(_test_func_, uniqid)(); \
		static __attribute__((unused)) testlib::TestSpec TEST3(_test_register_, uniqid)( \
	&TEST3(_test_func_, uniqid), desc, __FILE__, __LINE__, uniqid); \
		static void TEST3(_test_func_, uniqid)()
#define TEST(desc) TEST2(__COUNTER__, desc)

#define expect(expr) do { \
			if (!(expr)) { \
				throw testlib::TestFailure( \
	"Expected '" #expr "' to be true.", __FILE__, __LINE__); \
			} \
} while (0)

#define expecteq(a, b) do { \
			if ((a) != (b)) { \
				throw testlib::TestFailure( \
	"Expected '" #a "' to equal '" #b "'.", __FILE__, __LINE__); \
			} \
} while (0)

#define expectneq(a, b) do { \
			if ((a) == (b)) { \
				throw testlib::TestFailure( \
	"Expected '" #a "' to not equal '" #b "'.", __FILE__, __LINE__); \
			} \
} while (0)
