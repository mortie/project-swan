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
		if (!isInputAvailable(invContent, input)) {
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

void CraftingInventory::renderTooltip(
	Swan::Ctx &ctx, Cygnet::Renderer &rnd,
	Swan::Vec2 pos, int slot)
{
	auto inv = ref_.trait<Swan::InventoryTrait>();
	if (!inv) {
		return;
	}

	auto invContent = inv->content();
	if (slot < 0 || slot >= size() || size_t(slot) >= recipes_.size()) {
		return;
	}

	auto recipe = recipes_[slot];
	if (!recipe) {
		return;
	}

	// Compute maximum dynamic width,
	// and prepare texts at the same time
	float maxWidth = 0;
	segments_.clear();
	segments_.reserve(recipe->inputs.size());
	for (auto &input: recipe->inputs) {
		segments_.push_back(rnd.prepareUIText({
			.textCache = ctx.game.smallFont_,
			.pos{},
			.text = input.item()->displayName,
			.scale = 0.7,
		}));
		auto width = segments_.back().size.x;
		if (width > maxWidth) {
			maxWidth = width;
		}
	}

	// Prepare title
	auto title = rnd.prepareUIText({
		.textCache = ctx.game.smallFont_,
		.pos = pos.add(-0.25, 1),
		.text = Swan::cat(
			recipe->output.count(), "x ",
			recipe->output.item()->displayName),
		.scale = 0.7,
	});

	float backgroundWidth = maxWidth + 2.2;
	float backgroundHeight = recipe->inputs.size() * 0.8 + 1.25;
	if (backgroundWidth < title.size.x + 0.5) {
		backgroundWidth = title.size.x + 0.5;
	}

	// Render background
	rnd.drawUIRect({
		.pos = pos.add(
			backgroundWidth / 2 - 0.5,
			backgroundHeight / 2.0 + 0.55),
		.size = Swan::Vec2(backgroundWidth, backgroundHeight),
		.outline = {0.3, 0.1, 0.1},
		.fill = {0, 0, 0, 0.7},
	});

	// Render title text
	title.drawText.pos.x += title.size.x / 2;
	rnd.drawUIText(title);

	float y = 2;
	int i = 0;
	char buf[8];
	for (auto &input: recipe->inputs) {
		bool isAvailable = isInputAvailable(invContent, input);

		Cygnet::Color color = {1, 1, 1};
		if (!isAvailable) {
			color = {0.9, 0.2, 0.1};
		}

		snprintf(buf, sizeof(buf), "%dx", input.count());
		rnd.drawUIText({
			.textCache = ctx.game.smallFont_,
			.pos = pos.add(0, y),
			.text = buf,
			.scale = 0.7,
			.color = color,
		});

		rnd.drawUITile({
			.transform = Cygnet::Mat3gf{}
				.scale({0.7, 0.7})
				.translate(pos.add(1, y + 0.2)),
			.id = input.item()->id,
		});

		auto &segment = segments_[i++];
		segment.drawText.pos = pos.add(1.4, y - 0.15);
		segment.drawText.color = color;
		rnd.drawUIText(segment);
		y += 0.8;
	}
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
				if (discoveredRecipes_.contains(recipe.output.item()->id)) {
					content_.push_back({recipe.output.item(), -1});
					recipes_.push_back(&recipe);
				}

				continue;
			}

			discoveredRecipes_.insert(recipe.output.item()->id);
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

void CraftingInventory::serialize(
	const Swan::Context &ctx,
	proto::CraftingInventory::Builder w)
{
	auto recipes = w.initDiscoveredRecipes(discoveredRecipes_.size());
	int index = 0;
	for (auto id: discoveredRecipes_) {
		recipes.set(index++, ctx.world.getItemByID(id).name);
	}
}

void CraftingInventory::deserialize(
	const Swan::Context &ctx,
	proto::CraftingInventory::Reader r)
{
	for (auto x: r.getDiscoveredRecipes()) {
		auto &tile = ctx.world.getItem(x.cStr());
		if (tile.id == Swan::World::INVALID_TILE_ID) {
			continue;
		}

		discoveredRecipes_.insert(tile.id);
	}
}

bool CraftingInventory::isInputAvailable(
	std::span<const Swan::ItemStack> invContent,
	Swan::ItemStack input)
{
	int needed = input.count();
	for (auto &stack: invContent) {
		if (input.item() != stack.item()) {
			continue;
		}

		needed -= stack.count();
		if (needed <= 0) {
			return true;
		}
	}

	return false;
}

}
