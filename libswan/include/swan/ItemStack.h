#pragma once

#include "swan.capnp.h"

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

	Item *item() const
	{
		return item_;
	}

	int count() const
	{
		return count_;
	}

	bool empty() const
	{
		return item_ == nullptr;
	}

	bool full() const;

	// Insert as much of 'st' as possible, returning the leftovers
	ItemStack insert(ItemStack st);

	// Remove 'count' items from the stack, returning the removed items
	ItemStack remove(int count);

	void serialize(proto::ItemStack::Builder w);
	void deserialize(const Context &ctx, proto::ItemStack::Reader r);

	friend bool operator==(const ItemStack &a, const ItemStack &b)
	{
		if (a.empty() && b.empty()) {
			return true;
		}

		return a.item() == b.item() && a.count() == b.count();
	}

private:
	Item *item_;
	int count_;
};

}
