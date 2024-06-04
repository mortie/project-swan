#include "ItemPipeTileEntity.h"

#include <array>
#include <optional>

#include "entities/ItemStackEntity.h"

namespace CoreMod {

static constexpr std::array<Swan::Direction, 4> allDirections = {
	Swan::Direction::UP,
	Swan::Direction::DOWN,
	Swan::Direction::LEFT,
	Swan::Direction::RIGHT,
};

void ItemPipeTileEntity::tick(const Swan::Context &ctx, float dt)
{
	for (size_t i = 0; i < contents_.size();) {
		auto &item = contents_[i];
		item.timer += 1;
		if (item.timer >= 10) {
			moveItemOut(ctx, i);
		} else {
			i += 1;
		}
	}
}

void ItemPipeTileEntity::onDespawn(const Swan::Context &ctx)
{
	auto pos = tileEntity_.pos.as<float>().add(0.5, 0.5);
	for (auto &item: contents_) {
		ctx.plane.spawnEntity<ItemStackEntity>(pos, item.item);
	}
	for (auto &item: inbox_.contents_) {
		ctx.plane.spawnEntity<ItemStackEntity>(pos, item.item);
	}
}

void ItemPipeTileEntity::serialize(
	const Swan::Context &ctx, sbon::ObjectWriter w)
{
	// TODO
}

void ItemPipeTileEntity::deserialize(
	const Swan::Context &ctx, sbon::ObjectReader r)
{
	// TODO
}

void ItemPipeTileEntity::moveItemOut(const Swan::Context &ctx, size_t index)
{

	auto &item = contents_[index];
	for (auto dir: allDirections) {
		if (dir == item.from) {
			continue;
		}

		auto ref = ctx.plane.getTileEntity(tileEntity_.pos + dir);
		auto *inv = ref.trait<Swan::InventoryTrait>();
		if (!inv) {
			continue;
		}

		Swan::ItemStack stack(item.item, 1);
		stack = inv->insert(dir.opposite(), stack);
		if (!stack.empty()) {
			continue;
		}

		contents_[index] = contents_.back();
		contents_.pop_back();
		return;
	}

	// We found nowhere to put it,
	// so we must put it into the world
	auto dir = item.from.opposite().vec<float>();
	auto pos = tileEntity_.pos.as<float>().add(0.5, 0.5) + dir.scale(0.75);
	ctx.plane.spawnEntity<ItemStackEntity>(pos, dir.scale(5), item.item);
	contents_[index] = contents_.back();
	contents_.pop_back();
}

Swan::ItemStack ItemPipeTileEntity::Inbox::insert(Swan::ItemStack stack)
{
	int rand = Swan::random() % allDirections.size();
	return insert(allDirections[rand], stack);
}

Swan::ItemStack ItemPipeTileEntity::Inbox::insert(
	Swan::Direction from, Swan::ItemStack stack)
{
	auto oneItem = stack.remove(1);
	if (oneItem.empty()) {
		return stack;
	}

	contents_.push_back({oneItem.item(), from});
	return stack;
}

}
