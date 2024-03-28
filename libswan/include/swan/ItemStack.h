#pragma once

namespace Swan {

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
	[[nodiscard]] ItemStack insert(ItemStack st);

	// Remove 'count' items from the stack, returning the resulting stack
	[[nodiscard]] ItemStack remove(int count);

private:
	Item *item_;
	int count_;
};

}
