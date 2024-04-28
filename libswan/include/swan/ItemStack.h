#pragma once

#include <sbon.h>

namespace Swan {

struct Context;
struct Item;

class ItemStack {
public:
	ItemStack(Item *item, int count): item_(item), count_(count)
	{
		// We don't want a "partially empty" state.
		if (item == nullptr || count == 0) {
			item_ = nullptr;
			count_ = 0;
		}
	}

	ItemStack(): item_(nullptr), count_(0)
	{}

	Item *item()
	{
		return item_;
	}

	int count()
	{
		return count_;
	}

	bool empty()
	{
		return item_ == nullptr;
	}

	// Insert as much of 'st' as possible, returning the leftovers
	ItemStack insert(ItemStack st);

	// Remove 'count' items from the stack, returning the removed items
	ItemStack remove(int count);

	void serialize(sbon::Writer w);
	void deserialize(const Context &ctx, sbon::Reader r);

private:
	Item *item_;
	int count_;
};

}
