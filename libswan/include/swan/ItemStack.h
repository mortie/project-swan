#pragma once

namespace Swan {

class Item;

class ItemStack {
public:
	ItemStack(Item *item, int count): item_(item), count_(count) {

		// We don't want a "partially empty" state.
		if (item == nullptr || count == 0) {
			item_ = nullptr;
			count_ = 0;
		}
	}

	ItemStack(): item_(nullptr), count_(0) {}

	Item *item() { return item_; }
	int count() { return count_; }
	bool empty() { return item_ == nullptr; }

	// Insert as much of 'st' as possible, returning the leftovers
	ItemStack insert(ItemStack st);

private:
	Item *item_;
	int count_;
};

}
