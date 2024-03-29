#pragma once

#include <vector>

#include "../ItemStack.h"

namespace Swan {

struct InventoryTrait {
	struct Tag {};

	struct Inventory {
		virtual int size() = 0;
		virtual ItemStack get(int slot) = 0;
		virtual ItemStack set(int slot, ItemStack stack) = 0;
		virtual ItemStack insert(int slot, ItemStack stack) = 0;
		virtual ItemStack insert(ItemStack stack) = 0;

protected:
		~Inventory() = default;
	};

	virtual Inventory &get(Tag) = 0;

protected:
	~InventoryTrait() = default;
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
};

}
