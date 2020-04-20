#include "traits/InventoryTrait.h"

namespace Swan {

ItemStack InventoryTrait::BasicInventory::get(int slot) {
	if (slot >= (ssize_t)content.size())
		return ItemStack{};

	return content[slot];
}

ItemStack InventoryTrait::BasicInventory::set(int slot, ItemStack stack) {
	if (slot >= (ssize_t)content.size())
		return stack;

	ItemStack st = content[slot];
	content[slot] = stack;
	return st;
}

ItemStack InventoryTrait::BasicInventory::insert(int slot, ItemStack stack) {
	for (int i = 0; !stack.empty() && i < (ssize_t)content.size(); ++i)
		stack = content[i].insert(stack);
	return stack;
}

}
