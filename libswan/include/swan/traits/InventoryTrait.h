#pragma once

#include <vector>

#include "../Vector2.h"
#include "../ItemStack.h"

namespace Swan {

namespace InventoryTrait {

class Inventory;
class HasInventory {
public:
	virtual ~HasInventory() = default;
	virtual Inventory &getInventory() = 0;
};

class Inventory {
public:
	virtual ~Inventory() = default;

	virtual int size() = 0;
	virtual ItemStack &get(int slot) = 0;
	virtual ItemStack insert(ItemStack stack, int slot = 0) = 0;
};

class BasicInventory: public Inventory {
public:
	BasicInventory(int size): size_(size) {}

	int size() override { return size_; }

	ItemStack &get(int slot) {
		if (slot < size_)
			return content_[slot];
		return ItemStack();
	}

	ItemStack insert(ItemStack stack) {
		for (int i = 0; !stack.empty() && i < size_; ++i)
			stack = content_[i].insert(stack);
		return stack;
	}

private:
	int size_;
	std::vector<ItemStack> content_(size_);
};

}
}
