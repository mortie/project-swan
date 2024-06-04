#include "ItemPipeTileEntity.h"

#include <array>
#include <optional>

#include "entities/ItemStackEntity.h"

namespace CoreMod {

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

	if (inbox_.contents_ && contents_.size() < 10) {
		auto input = inbox_.contents_.value();
		inbox_.contents_.reset();

		Swan::DirectionSet possibilities;
		for (auto dir: Swan::DirectionSet::all()) {
			if (dir == input.from) {
				continue;
			}

			auto pos = tileEntity_.pos + dir;
			auto ent = ctx.plane.getTileEntity(pos);
			if (!ent) {
				continue;
			}

			auto inv = ent.trait<Swan::InventoryTrait>();
			if (!inv) {
				continue;
			}

			possibilities.set(dir);
			break;
		}

		auto dest = possibilities.random();
		if (!dest) {
			dest = input.from.opposite();
		}

		contents_.push_back({
			.item = input.item,
			.from = input.from,
			.to = dest.value(),
		});
	}
}

void ItemPipeTileEntity::moveItemOut(const Swan::Context &ctx, size_t index)
{
	auto item = contents_[index];
	contents_[index] = contents_.back();
	contents_.pop_back();

	auto pos = tileEntity_.pos + item.to;
	bool ok = [&] {
		auto ent = ctx.plane.getTileEntity(pos);
		if (!ent) {
			return false;
		}

		auto inv = ent.trait<Swan::InventoryTrait>();
		if (!inv) {
			return false;
		}

		Swan::ItemStack stack(item.item, 1);
		stack = inv->insert(item.to, stack);
		return stack.empty();
	}();

	if (!ok) {
		auto entPos = pos.as<float>().add(0.5, 0.5);
		ctx.plane.spawnEntity<ItemStackEntity>(entPos, item.item);
	}
}

void ItemPipeTileEntity::onDespawn(const Swan::Context &ctx)
{
	auto pos = tileEntity_.pos.as<float>().add(0.5, 0.5);
	for (auto &item: contents_) {
		ctx.plane.spawnEntity<ItemStackEntity>(pos, item.item);
	}

	if (inbox_.contents_) {
		ctx.plane.spawnEntity<ItemStackEntity>(pos, inbox_.contents_.value().item);
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

Swan::ItemStack ItemPipeTileEntity::Inbox::insert(Swan::ItemStack stack)
{
	return insert(Swan::Direction::random(), stack);
}

Swan::ItemStack ItemPipeTileEntity::Inbox::insert(
	Swan::Direction from, Swan::ItemStack stack)
{
	if (contents_) {
		return stack;
	}

	auto oneItem = stack.remove(1);
	if (oneItem.empty()) {
		return stack;
	}

	contents_ = {oneItem.item(), from};
	return stack;
}

}
