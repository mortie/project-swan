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

void BasicInventory::serialize(proto::BasicInventory::Builder w)
{
	w.setSize(content.size());

	size_t filledSlots = 0;
	for (auto &slot: content) {
		if (!slot.empty()) {
			filledSlots += 1;
		}
	}

	auto slotsW = w.initSlots(filledSlots);
	for (size_t i = 0; i < content.size(); ++i) {
		if (content[i].empty()) {
			continue;
		}

		slotsW[i].setIndex(i);
		content[i].serialize(slotsW[i].initStack());
	}
}

void BasicInventory::deserialize(const Swan::Context &ctx, proto::BasicInventory::Reader r)
{
	content.clear();
	content.resize(r.getSize());

	for (auto slot: r.getSlots()) {
		content[slot.getIndex()].deserialize(ctx, slot.getStack());
	}
}

}
