#pragma once

#include <swan/swan.h>
#include <vector>

namespace CoreMod {

class CraftingInventory final: public Swan::InventoryTrait::Inventory {
public:
	CraftingInventory(Swan::EntityRef ref): ref_(ref)
	{}

	Swan::ItemStack take(int slot) override;

	std::span<const Swan::ItemStack> content() const override
	{
		return content_;
	}

	Swan::ItemStack set(int slot, Swan::ItemStack stack) override
	{
		return stack;
	}

	Swan::ItemStack insertInto(Swan::ItemStack stack, int from, int to) override
	{
		return stack;
	}

	void recompute(const Swan::Context &ctx, std::span<const Swan::ItemStack> items);

private:
	std::vector<Swan::ItemStack> content_;
	std::vector<Swan::Recipe *> recipes_;
	Swan::EntityRef ref_;
	std::unordered_map<Swan::Item *, int> itemCounts_;
};

}
