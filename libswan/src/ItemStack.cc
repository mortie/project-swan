#include "ItemStack.h"

#include "Item.h"
#include "World.h"

namespace Swan {

bool ItemStack::full() const
{
	if (empty()) {
		return false;
	}

	return count_ >= item_->maxStack;
}

ItemStack ItemStack::insert(ItemStack st)
{
	// If this is an empty item stack, just copy over st
	if (empty()) {
		item_ = st.item_;
		count_ = st.count_;
		st.item_ = nullptr;
		st.count_ = 0;
		return st;
	}

	// If st is a stack of a different kind of item, we don't want it
	if (st.item_ != item_) {
		return st;
	}

	// Merge
	count_ += st.count_;
	if (count_ > item_->maxStack) {
		st.count_ = count_ - item_->maxStack;
		count_ = item_->maxStack;
	}
	else {
		st.count_ = 0;
		st.item_ = nullptr;
	}

	return st;
}

ItemStack ItemStack::remove(int count)
{
	if (empty()) {
		return ItemStack();
	}

	if (count > count_) {
		count = count_;
	}

	ItemStack newStack(item_, count);
	count_ -= count;
	if (count_ == 0) {
		item_ = nullptr;
	}

	return newStack;
}

void ItemStack::serialize(proto::ItemStack::Builder w)
{
	if (empty()) {
		return;
	}

	w.setCount(count_);
	w.setItem(item_->name);
}

void ItemStack::deserialize(const Swan::Context &ctx, proto::ItemStack::Reader r)
{
	if (!r.hasItem()) {
		count_ = 0;
		item_ = nullptr;
		return;
	}

	count_ = r.getCount();
	item_ = &ctx.world.getItem(r.getItem().cStr());
}

}
