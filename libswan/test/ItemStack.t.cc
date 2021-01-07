#include "ItemStack.h"
#include "Item.h"

#include "lib/test.h"

class MockItem: public Swan::Item {
public:
	MockItem(const Builder &builder): Item(builder) {}
};

test("Basic insert") {
	MockItem item1({ .name = "item1", .image = "no" });

	Swan::ItemStack s1(&item1, 0);
	Swan::ItemStack s2(&item1, 10);
	s2 = s1.insert(s2);

	expecteq(s1.count(), 10);
	expecteq(s2.count(), 0);
}

test("Insert rejects different items") {
	MockItem item1({ .name = "item1", .image = "no" });
	MockItem item2({ .name = "itemm2", .image = "no" });

	Swan::ItemStack s1(&item1, 5);
	Swan::ItemStack s2(&item2, 10);
	Swan::ItemStack ret = s1.insert(s2);

	expecteq(s1.count(), 5);
	expecteq(ret.count(), 10);
	expecteq(ret.count(), s2.count());
	expecteq(ret.item(), s2.item());
}

test("Insert never overflows") {
	MockItem item1({ .name = "item1", .image = "no" });

	Swan::ItemStack s1(&item1, 40);
	Swan::ItemStack s2(&item1, 40);
	s2 = s1.insert(s2);

	expecteq(s1.count(), item1.maxStack_);
	expecteq(s2.count(), 80 - item1.maxStack_);
}

test("Insert respects max_stack_") {
	MockItem item1({ .name = "item1", .image = "no", .maxStack = 20 });

	Swan::ItemStack s1(&item1, 15);
	Swan::ItemStack s2(&item1, 19);
	s2 = s1.insert(s2);

	expecteq(s1.count(), 20);
	expecteq(s2.count(), 14);
}

test("When insert empties an ItemStack, it should have its item nulled out") {
	MockItem item1({ .name = "item1", .image = "no" });
	MockItem item2({ .name = "itemm2", .image = "no" });

	Swan::ItemStack s1(&item1, 10);
	Swan::ItemStack s2(&item1, 10);
	s2 = s1.insert(s2);

	expecteq(s1.count(), 20);
	expecteq(s2.count(), 0);
	expecteq(s2.item(), nullptr);
	expect(s2.empty());
}

test("Insert on an empty item stack") {
	MockItem item1({ .name = "item1", .image = "no" });

	Swan::ItemStack s1(nullptr, 0);
	Swan::ItemStack s2(&item1, 10);
	s2 = s1.insert(s2);

	expecteq(s1.count(), 10);
	expecteq(s1.item(), &item1);
	expecteq(s2.count(), 0);
	expect(s2.empty());
}
