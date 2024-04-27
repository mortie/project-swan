#include "traits/InventoryTrait.h"
#include "Item.h"
#include "World.h"

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

void BasicInventory::serialize(nbon::Writer w)
{
	w.writeArray([&](nbon::Writer w) {
		for (auto &stack: content) {
			if (stack.empty()) {
				w.writeNull();
			}
			else {
				w.writeArray([&](nbon::Writer w) {
					w.writeString(stack.item()->name);
					w.writeUInt(stack.count());
				});
			}
		}
	});
}

void BasicInventory::deserialize(const Swan::Context &ctx, nbon::Reader r)
{
	content.clear();
	r.readArray([&](nbon::Reader r) {
		if (r.getType() == nbon::Type::NIL) {
			r.getNil();
			content.emplace_back();
		} else {
			r.getArray([&](nbon::ArrayReader r) {
				auto name = r.next().getString();
				auto count = r.next().getUInt();
				content.emplace_back(&ctx.world.getItem(name), count);
			});
		}
	});
}

}
