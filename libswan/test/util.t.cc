#include "util.h"

#include "lib/test.h"

test("map") {
	int ints[] = { 100, 200, 300, 400 };
	auto mapping = Swan::map(ints, [](int i) { return i / 10; });
	auto iter = mapping.begin();

	expecteq(*iter, 10); ++iter;
	expecteq(*iter, 20); ++iter;
	expecteq(*iter, 30); ++iter;
	expecteq(*iter, 40); ++iter;
	expecteq(iter, mapping.end());
}

test("filter") {
	int ints[] = { 100, 200, 300, 400 };
	auto filter = Swan::filter(ints, [](int i) { return i == 200 || i == 400; });
	auto iter = filter.begin();

	expecteq(*iter, 200); ++iter;
	expecteq(*iter, 400); ++iter;
	expecteq(iter, filter.end());
}

test("mapFilter") {
	float floats[] = { 10.1, 20.2, 30.3 };
	auto mapfilt = Swan::mapFilter(floats, [](float f) -> std::optional<int> {
		if ((int)f == 20)
			return std::nullopt;
		return (int)f;
	});
	auto iter = mapfilt.begin();

	expecteq(*iter, 10); ++iter;
	expecteq(*iter, 30); ++iter;
	expecteq(iter, mapfilt.end());
}
