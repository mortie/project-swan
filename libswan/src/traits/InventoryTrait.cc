#include "traits/InventoryTrait.h"

namespace Swan {

ItemStack BasicInventory::get(int slot)
{
	if ((size_t)slot >= content.size()) {
		return ItemStack{};
	}

	return content[slot];
}

ItemStack BasicInventory::set(int slot, ItemStack stack)
{
	if ((size_t)slot >= content.size()) {
		return stack;
	}

	ItemStack st = content[slot];
	content[slot] = stack;
	return st;
}

ItemStack BasicInventory::insert(int slot, ItemStack stack)
{
	for (int i = 0; !stack.empty() && (size_t)i < content.size(); ++i) {
		stack = content[i].insert(stack);
	}
	return stack;
}

}
