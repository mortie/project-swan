#include "ItemStack.h"

#include "lib/test.h"

// Most of these tests need two ItemStacks.
// ItemStack requires an Item pointer, and only ever looks at the pointer.
// Therefore, we'll just give the ItemStacks pointers to ints.
static int itint1, itint2;
static Swan::Item *item1 = (Swan::Item *)&itint1;
static Swan::Item *item2 = (Swan::Item *)&itint2;

test("Basic insert") {
	Swan::ItemStack s1(item1, 0);
	Swan::ItemStack s2(item1, 10);
	s2 = s1.insert(s2);

	expecteq(s1.count(), 10);
	expecteq(s2.count(), 0);
}

test("Insert rejects different items") {
	Swan::ItemStack s1(item1, 5);
	Swan::ItemStack s2(item2, 10);
	Swan::ItemStack ret = s1.insert(s2);

	expecteq(s1.count(), 5);
	expecteq(ret.count(), 10);
	expecteq(ret.count(), s2.count());
	expecteq(ret.item(), s2.item());
}

test("Insert never overflows") {
	Swan::ItemStack s1(item1, 40);
	Swan::ItemStack s2(item1, 40);
	s2 = s1.insert(s2);

	expecteq(s1.count(), Swan::ItemStack::MAX_COUNT);
	expecteq(s2.count(), 80 - Swan::ItemStack::MAX_COUNT);
}


test("When insert empties an ItemStack, it should have its item nulled out") {
	Swan::ItemStack s1(item1, 10);
	Swan::ItemStack s2(item1, 10);
	s2 = s1.insert(s2);

	expecteq(s1.count(), 20);
	expecteq(s2.count(), 0);
	expecteq(s2.item(), nullptr);
	expect(s2.empty());
}

test("Insert on an empty item stack") {
	Swan::ItemStack s1(nullptr, 0);
	Swan::ItemStack s2(item1, 10);
	s2 = s1.insert(s2);

	expecteq(s1.count(), 10);
	expecteq(s1.item(), item1);
	expecteq(s2.count(), 0);
	expect(s2.empty());
}
