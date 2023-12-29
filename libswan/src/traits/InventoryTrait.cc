#include "traits/InventoryTrait.h"
#include "log.h"

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
	return content[slot].insert(stack);
}

ItemStack BasicInventory::insert(ItemStack stack)
{
	int s = size();

	// First try to insert into an existing stack
	for (int i = 0; i < s && !stack.empty(); ++i) {
		if (content[i].item() == stack.item()) {
			stack = content[i].insert(stack);
		}
	}

	// Then find a new slot
	for (int i = 0; i < s && !stack.empty(); ++i) {
		stack = content[i].insert(stack);
	}

	return stack;
}

}
