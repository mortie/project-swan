#include "util.h"

#include "lib/test.h"

test("Deferred functions run") {
	int runs = 0;
	{
		auto def = Swan::makeDeferred([&] { runs += 1; });
		expecteq(runs, 0);
	}
	expecteq(runs, 1);
}

test("Moved deferreds don't run") {
	int runs = 0;
	{
		auto def = Swan::makeDeferred([&] { runs += 1; });
		auto def2 = std::move(def);
		expecteq(runs, 0);
	}
	expecteq(runs, 1);
}

test("Raii pointers") {
	int runs = 0;
	{
		auto ptr = Swan::makeRaiiPtr(new int(10), [&](int *p) {
			runs += 1;
			delete p;
		});
		expecteq(runs, 0);
		expectneq(ptr, nullptr);
		expecteq(*ptr, 10);
	}

	expecteq(runs, 1);
}
