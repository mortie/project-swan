#pragma once

#include <span>
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
		virtual ItemStack take(int slot) = 0;
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

		virtual std::span<const ItemStack> content() const = 0;

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

	ItemStack take()
	{
		return inventory->take(slot);
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

class BasicInventory final: public InventoryTrait::Inventory {
public:
	BasicInventory(int size): content_(size)
	{}

	std::span<const ItemStack> content() const override
	{
		return content_;
	}

	int size() override
	{
		return content_.size();
	}

	ItemStack get(int slot) override;
	ItemStack take(int slot) override;
	ItemStack set(int slot, ItemStack stack) override;
	ItemStack insert(int slot, ItemStack stack) override;
	ItemStack insert(ItemStack stack) override;

	void serialize(proto::BasicInventory::Builder w);
	void deserialize(const Swan::Context &ctx, proto::BasicInventory::Reader r);

	std::vector<ItemStack> content_;
};

inline InventorySlot InventoryTrait::Inventory::slot(int slot)
{
	return {this, slot};
}

}
