#pragma once

#include <vector>

#include "../ItemStack.h"
#include "../common.h"
#include "../Direction.h"
#include "swan.capnp.h"

namespace Swan {

struct InventorySlot;
struct Context;

struct InventoryTrait {
	struct Tag {};

	struct Inventory {
		virtual int size() = 0;
		virtual ItemStack get(int slot) = 0;
		virtual ItemStack set(int slot, ItemStack stack) = 0;
		virtual ItemStack insert(ItemStack stack) = 0;

		virtual ItemStack insert(int slot, ItemStack stack)
		{
			return insert(stack);
		}

		virtual ItemStack insert(Direction dir, ItemStack stack)
		{
			return insert(stack);
		}

		InventorySlot slot(int slot);

	protected:
		~Inventory() = default;
	};

	virtual Inventory &get(Tag) = 0;

protected:
	~InventoryTrait() = default;
};

struct InventorySlot {
	InventoryTrait::Inventory *inventory;
	const int slot;

	ItemStack get()
	{
		return inventory->get(slot);
	}

	ItemStack set(ItemStack stack)
	{
		return inventory->set(slot, stack);
	}

	ItemStack insert(ItemStack stack)
	{
		return inventory->insert(slot, stack);
	}

	ItemStack remove(int n)
	{
		auto stack = get();
		auto ret = stack.remove(1);
		set(stack);
		return ret;
	}
};

struct BasicInventory final: InventoryTrait::Inventory {
	BasicInventory(int size): content(size)
	{}

	std::vector<ItemStack> content;

	int size() override
	{
		return content.size();
	}

	ItemStack get(int slot) override;
	ItemStack set(int slot, ItemStack stack) override;
	ItemStack insert(int slot, ItemStack stack) override;
	ItemStack insert(ItemStack stack) override;

	void serialize(proto::BasicInventory::Builder w);
	void deserialize(const Swan::Context &ctx, proto::BasicInventory::Reader r);
};

inline InventorySlot InventoryTrait::Inventory::slot(int slot)
{
	return {this, slot};
}

}
