#include "traits/InventoryTrait.h"

namespace Swan {

ItemStack BasicInventory::take(int slot)
{
	if ((size_t)slot >= content_.size()) {
		return ItemStack{};
	}

	ItemStack stack = content_[slot];
	content_[slot] = {};
	return stack;
}

ItemStack BasicInventory::set(int slot, ItemStack stack)
{
	if ((size_t)slot >= content_.size()) {
		return stack;
	}

	ItemStack st = content_[slot];
	content_[slot] = stack;
	return st;
}

ItemStack BasicInventory::insertInto(ItemStack stack, int from, int to)
{
	if (to < 0 || to > size()) {
		to = size();
	}

	// First try to insert into an existing stack
	for (int i = from; i < to && !stack.empty(); ++i) {
		if (content_[i].item() == stack.item()) {
			stack = content_[i].insert(stack);
		}
	}

	// Then find a new slot
	for (int i = from; i < to && !stack.empty(); ++i) {
		stack = content_[i].insert(stack);
	}

	return stack;
}

void BasicInventory::serialize(proto::BasicInventory::Builder w)
{
	w.setSize(content_.size());

	size_t filledSlots = 0;
	for (auto &slot: content_) {
		if (!slot.empty()) {
			filledSlots += 1;
		}
	}

	auto slotsW = w.initSlots(filledSlots);
	size_t index = 0;
	for (size_t i = 0; i < content_.size(); ++i) {
		if (content_[i].empty()) {
			continue;
		}

		slotsW[index].setIndex(i);
		content_[i].serialize(slotsW[index].initStack());
		index += 1;
	}
}

void BasicInventory::deserialize(const Swan::Context &ctx, proto::BasicInventory::Reader r)
{
	content_.clear();
	content_.resize(r.getSize());

	for (auto slot: r.getSlots()) {
		content_[slot.getIndex()].deserialize(ctx, slot.getStack());
	}
}

}
