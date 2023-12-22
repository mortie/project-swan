#include "ItemStack.h"
#include "Item.h"

#include "lib/test.h"

using namespace Swan;

TEST("Basic insert") {
	Item item1(0, "test::item1", {});

	ItemStack s1(&item1, 0);
	ItemStack s2(&item1, 10);
	s2 = s1.insert(s2);

	expecteq(s1.count(), 10);
	expecteq(s2.count(), 0);
}

TEST("Insert rejects different items") {
	Item item1(0, "test::item1", {});
	Item item2(1, "test::item2", {});

	ItemStack s1(&item1, 5);
	ItemStack s2(&item2, 10);
	ItemStack ret = s1.insert(s2);

	expecteq(s1.count(), 5);
	expecteq(ret.count(), 10);
	expecteq(ret.count(), s2.count());
	expecteq(ret.item(), s2.item());
}

TEST("Insert never overflows") {
	Item item1(0, "test::item1", {});

	ItemStack s1(&item1, 40);
	ItemStack s2(&item1, 40);
	s2 = s1.insert(s2);

	expecteq(s1.count(), item1.maxStack);
	expecteq(s2.count(), 80 - item1.maxStack);
}

TEST("Insert respects maxStack") {
	Item item1(0, "test::item1", {.maxStack = 20});

	Swan::ItemStack s1(&item1, 15);
	Swan::ItemStack s2(&item1, 19);
	s2 = s1.insert(s2);

	expecteq(s1.count(), 20);
	expecteq(s2.count(), 14);
}

TEST("When insert empties an ItemStack, it should have its item nulled out") {
	Item item1(0, "test::item1", {});
	Item item2(1, "test::item2", {});

	ItemStack s1(&item1, 10);
	ItemStack s2(&item1, 10);
	s2 = s1.insert(s2);

	expecteq(s1.count(), 20);
	expecteq(s2.count(), 0);
	expecteq(s2.item(), nullptr);
	expect(s2.empty());
}

TEST("Insert on an empty item stack") {
	Item item1(0, "test::item1", {});

	ItemStack s1(nullptr, 0);
	ItemStack s2(&item1, 10);
	s2 = s1.insert(s2);

	expecteq(s1.count(), 10);
	expecteq(s1.item(), &item1);
	expecteq(s2.count(), 0);
	expect(s2.empty());
}
