#pragma once

#include <vector>
#include <stdlib.h>

#include "../Vector2.h"
#include "../ItemStack.h"

namespace Swan {

namespace InventoryTrait {

class Inventory;
class HasInventory {
public:
	virtual Inventory &getInventory() = 0;
};

class Inventory {
public:
	virtual ~Inventory() = default;

	virtual int size() = 0;
	virtual ItemStack get(int slot) = 0;
	virtual ItemStack set(int slot, ItemStack stack) = 0;
	virtual ItemStack insert(int slot, ItemStack stack) = 0;

	ItemStack insert(ItemStack stack) { return insert(0, stack); }
};

class BasicInventory: public Inventory {
public:
	BasicInventory(int size): size_(size), content_(size) {}

	int size() override { return size_; }

	ItemStack get(int slot) override {
		if (slot < size_)
			return content_[slot];
		return ItemStack();
	}

	ItemStack set(int slot, ItemStack stack) override {
		ItemStack st = content_[slot];
		content_[slot] = stack;
		return st;
	}

	ItemStack insert(int slot, ItemStack stack) override {
		for (int i = 0; !stack.empty() && i < size_; ++i)
			stack = content_[i].insert(stack);
		return stack;
	}

private:
	int size_;
	std::vector<ItemStack> content_;
};

}
}
