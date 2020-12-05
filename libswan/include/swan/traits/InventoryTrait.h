#pragma once

#include <vector>
#include <stdlib.h>

#include "../common.h"
#include "../ItemStack.h"

namespace Swan {

struct InventoryTrait {
	struct Inventory;
	struct Tag {};
	virtual Inventory &get(Tag) = 0;

	struct Inventory {
		virtual ~Inventory() = default;

		virtual int size() = 0;
		virtual ItemStack get(int slot) = 0;
		virtual ItemStack set(int slot, ItemStack stack) = 0;
		virtual ItemStack insert(int slot, ItemStack stack) = 0;

		ItemStack insert(ItemStack stack) { return insert(0, stack); }
	};

	struct BasicInventory: Inventory {
		BasicInventory(int size): content(size) {}

		std::vector<ItemStack> content;

		int size() override { return content.size(); }
		ItemStack get(int slot) override;
		ItemStack set(int slot, ItemStack stack) override;
		ItemStack insert(int slot, ItemStack stack) override;
	};
};

}
