#pragma once

#include <optional>

#include "log.h"

namespace Swan {

class Item;

class ItemStack {
public:
	static constexpr int MAX_COUNT = 64;

	ItemStack(Item *item, int count = 0): item_(item), count_(count) {

		// We don't want a "partially empty" state.
		if (item == nullptr || count == 0) {
			item = nullptr;
			count = 0;
		}
	}

	Item *item() { return item_; }
	int count() { return count_; }
	bool empty() { return item_ == nullptr; }

	// Insert as much of 'st' as possible, returning the leftovers
	ItemStack insert(ItemStack st) {

		// If this is an empty item stack, just copy over st
		if (empty()) {
			item_ = st.item_;
			count_ = st.count_;
			st.item_ = nullptr;
			st.count_ = 0;
			return st;
		}

		// If st is a stack of a different kind of item, we don't want it
		if (st.item_ != item_)
			return st;

		// Merge
		count_ += st.count_;
		if (count_ > MAX_COUNT) {
			st.count_ = count_ - MAX_COUNT;
			count_ = MAX_COUNT;
		} else {
			st.count_ = 0;
			st.item_ = nullptr;
		}

		return st;
	}

private:
	Item *item_;
	int count_;
};

}
