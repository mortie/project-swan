#include "traits/InventoryTrait.h"
#include "log.h"
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

void BasicInventory::serialize(MsgStream::Serializer &w)
{
	auto arr = w.beginArray(content.size());

	for (auto &stack: content) {
		if (stack.empty()) {
			arr.writeNil();
		}
		else {
			auto stackArr = arr.beginArray(2);
			stackArr.writeString(stack.item()->name);
			stackArr.writeUInt(stack.count());
			arr.endArray(stackArr);
		}
	}

	w.endArray(arr);
}

void BasicInventory::deserialize(const Swan::Context &ctx, MsgStream::Parser &r)
{
	auto arr = r.nextArray();

	content.clear();
	content.reserve(arr.arraySize());
	while (arr.hasNext()) {
		auto nextType = arr.nextType();
		if (nextType == MsgStream::Type::NIL) {
			arr.skipNil();
			content.emplace_back();
		}
		else {
			auto stackArr = arr.nextArray();
			std::string itemName = stackArr.nextString();
			int count = (int)stackArr.nextInt();
			stackArr.skipAll();
			content.emplace_back(&ctx.world.getItem(itemName), (int)count);
		}
	}
}

}
