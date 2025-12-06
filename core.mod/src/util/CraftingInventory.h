#pragma once

#include <swan/swan.h>
#include <vector>

namespace CoreMod {

class CraftingInventory final: public Swan::InventoryTrait::Inventory {
public:
	CraftingInventory(Swan::EntityRef ref): ref_(ref)
	{}

	struct Options {
		bool workbench = false;
	};

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

	void renderTooltip(
		Swan::Ctx &ctx, Cygnet::Renderer &rnd,
		Swan::Vec2 pos, int slot) override;

	void recompute(
		Swan::Ctx &ctx,
		std::span<const Swan::ItemStack> items,
		Options options);

	void clear()
	{
		content_.clear();
		recipes_.clear();
		availableRecipes_.clear();
	}

private:
	std::vector<Swan::ItemStack> content_;
	std::vector<Swan::Recipe *> recipes_;
	std::vector<Cygnet::Renderer::TextSegment> segments_;
	std::unordered_set<Swan::Tile::ID> availableRecipes_;
	Swan::EntityRef ref_;
	std::unordered_map<Swan::Item *, int> itemCounts_;
};

}
