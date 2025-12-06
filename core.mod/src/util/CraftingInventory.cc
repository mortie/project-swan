#include "util/CraftingInventory.h"

namespace CoreMod {

Swan::ItemStack CraftingInventory::take(int slot)
{
	auto inv = ref_.trait<Swan::InventoryTrait>();
	if (!inv) {
		Swan::warn << "Taking from CraftingInventory without a reference";
		return {};
	}

	auto invContent = inv->content();
	if (slot < 0 || slot >= size()) {
		return {};
	}

	// Verify item availability
	for (auto &input: recipes_[slot]->inputs) {
		int needed = input.count();
		for (auto &stack: invContent) {
			if (input.item() != stack.item()) {
				continue;
			}

			needed -= stack.count();
			if (needed <= 0) {
				break;
			}
		}

		if (needed > 0) {
			return {};
		}
	}

	// Take items
	for (auto &input: recipes_[slot]->inputs) {
		int needed = input.count();
		for (int i = 0; i < int(invContent.size()); ++i) {
			auto stack = invContent[i];
			if (stack.item() != input.item()) {
				continue;
			}

			if (stack.count() >= needed) {
				stack.remove(needed);
				inv->set(i, stack);
				needed = 0;
				break;
			} else {
				needed -= stack.count();
				inv->set(i, {});
			}
		}
	}

	// Return the item
	return recipes_[slot]->output;
}

void CraftingInventory::recompute(
	Swan::Ctx &ctx,
	std::span<const Swan::ItemStack> items,
	Options options)
{
	for (auto &[k, v]: itemCounts_) {
		v = 0;
	}

	for (auto &stack: items) {
		if (stack.empty()) {
			continue;
		}

		itemCounts_[stack.item()] += stack.count();
	}

	content_.clear();
	recipes_.clear();

	auto iterateRecipes = [this, ctx](std::string_view kind) {
		for (auto &recipe: ctx.world.getRecipes(kind)) {
			bool craftable = true;
			for (const auto &input: recipe.inputs) {
				auto it = itemCounts_.find(input.item());
				if (it == itemCounts_.end() || it->second < input.count()) {
					craftable = false;
					break;
				}
			}

			if (!craftable) {
				if (availableRecipes_.contains(recipe.output.item()->id)) {
					content_.push_back({recipe.output.item(), -1});
					recipes_.push_back(&recipe);
				}

				continue;
			}

			availableRecipes_.insert(recipe.output.item()->id);
			content_.push_back({recipe.output.item(), recipe.output.count()});
			recipes_.push_back(&recipe);
		}
	};

	iterateRecipes("core::crafting");
	if (options.workbench) {
		iterateRecipes("core::workbench");
	}

	if (content_.size() == 0) {
		content_.push_back({});
		recipes_.push_back({});
	}
}

}
