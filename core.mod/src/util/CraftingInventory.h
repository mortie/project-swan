#pragma once

#include <swan/swan.h>
#include <vector>

#include "core_mod.capnp.h"

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

	void serialize(
		const Swan::Context &ctx,
		proto::CraftingInventory::Builder w);
	void deserialize(
		const Swan::Context &ctx,
		proto::CraftingInventory::Reader r);

private:
	bool isInputAvailable(
		std::span<const Swan::ItemStack> invContent,
		Swan::ItemStack input);

	std::vector<Swan::ItemStack> content_;
	std::vector<Swan::Recipe *> recipes_;
	std::vector<Cygnet::Renderer::TextSegment> segments_;
	std::unordered_set<Swan::Tile::ID> discoveredRecipes_;
	Swan::EntityRef ref_;
	std::unordered_map<Swan::Item *, int> itemCounts_;
};

}
