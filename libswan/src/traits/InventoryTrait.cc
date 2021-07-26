#include "traits/InventoryTrait.h"

namespace Swan {

ItemStack BasicInventory::get(int slot) {
	if (slot >= (ssize_t)content.size())
		return ItemStack{};

	return content[slot];
}

ItemStack BasicInventory::set(int slot, ItemStack stack) {
	if (slot >= (ssize_t)content.size())
		return stack;

	ItemStack st = content[slot];
	content[slot] = stack;
	return st;
}

ItemStack BasicInventory::insert(int slot, ItemStack stack) {
	for (int i = 0; !stack.empty() && i < (ssize_t)content.size(); ++i)
		stack = content[i].insert(stack);
	return stack;
}

}
