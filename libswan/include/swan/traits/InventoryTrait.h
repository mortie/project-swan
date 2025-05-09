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
		int size()
		{
			return content().size();
		}

		ItemStack get(int slot)
		{
			auto c = content();
			if (slot < 0 || slot >= int(c.size())) {
				return {};
			}

			return c[slot];
		}

		virtual ItemStack take(int slot) = 0;
		virtual ItemStack set(int slot, ItemStack stack) = 0;
		virtual ItemStack insertInto(ItemStack stack, int from, int to) = 0;

		virtual ItemStack insertSided(Direction dir, ItemStack stack)
		{
			return insert(stack);
		}

		InventorySlot slot(int slot);

		virtual std::span<const ItemStack> content() const = 0;

		ItemStack insert(ItemStack stack, int from = 0)
		{
			return insertInto(stack, from, size());
		}

		ItemStack insert(ItemStack stack, int from, int to)
		{
			return insertInto(stack, from, size());
		}

		ItemStack insert(Direction dir, ItemStack stack)
		{
			return insertSided(dir, stack);
		}

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
		return inventory->insert(stack, slot, slot + 1);
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

	ItemStack take(int slot) override;
	ItemStack set(int slot, ItemStack stack) override;
	ItemStack insertInto(ItemStack stack, int from, int to) override;

	void serialize(proto::BasicInventory::Builder w);
	void deserialize(const Swan::Context &ctx, proto::BasicInventory::Reader r);

	std::vector<ItemStack> content_;
};

inline InventorySlot InventoryTrait::Inventory::slot(int slot)
{
	return {this, slot};
}

}
