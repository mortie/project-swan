#include "ItemPipeTileEntity.h"

#include <optional>

#include "entities/ItemStackEntity.h"

namespace CoreMod {

void ItemPipeTileEntity::draw(const Swan::Context &ctx, Cygnet::Renderer &rnd)
{
	Swan::Vec2 center = tileEntity_.pos.as<float>().add(0.5, 0.5);

	for (auto &item: content_) {
		float frac;
		Swan::Vec2 from;
		Swan::Vec2 to;
		auto timer = item.timer + 1;
		if (timer <= 5) {
			frac = timer / 5.0;
			from = (center + (item.from.vec().as<float>() * 0.5));
			to = center;
		} else {
			frac = (timer - 5) / 5.0;
			from = center;
			to = (center + (item.to.vec().as<float>() * 0.5));;
		}

		Swan::Vec2 pos = Swan::lerp(from, to, frac).add(-0.25, -0.25);
		rnd.drawTile(Cygnet::RenderLayer::BEHIND, {
			Cygnet::Mat3gf{}.scale({0.5, 0.5}).translate(pos),
			item.item->id, 0.8,
		});
	}
}

void ItemPipeTileEntity::tick(const Swan::Context &ctx, float dt)
{
	for (size_t i = 0; i < content_.size();) {
		auto &item = content_[i];
		item.timer += 1;
		if (item.timer >= 10) {
			moveItemOut(ctx, i);
		} else {
			i += 1;
		}
	}
}

void ItemPipeTileEntity::tick2(const Swan::Context &ctx, float dt)
{
	if (inbox_.content_ && content_.size() < 10) {
		auto input = inbox_.content_.value();
		inbox_.content_.reset();

		Swan::DirectionSet possibilities;
		for (auto dir: Swan::DirectionSet::all()) {
			if (dir == input.from) {
				continue;
			}

			auto pos = tileEntity_.pos + dir;
			auto ent = ctx.plane.entities().getTileEntity(pos);
			if (!ent) {
				continue;
			}

			auto inv = ent.trait<Swan::InventoryTrait>();
			if (!inv) {
				continue;
			}

			possibilities.set(dir);
		}

		auto dest = possibilities.random();
		if (!dest) {
			dest = input.from.opposite();
		}

		content_.push_back({
			.item = input.item,
			.from = input.from,
			.to = dest.value(),
		});
	}
}

void ItemPipeTileEntity::moveItemOut(const Swan::Context &ctx, size_t index)
{
	auto item = content_[index];
	content_[index] = content_.back();
	content_.pop_back();

	auto pos = tileEntity_.pos + item.to;
	bool ok = [&] {
		auto ent = ctx.plane.entities().getTileEntity(pos);
		if (!ent) {
			return false;
		}

		auto inv = ent.trait<Swan::InventoryTrait>();
		if (!inv) {
			return false;
		}

		Swan::ItemStack stack(item.item, 1);
		stack = inv->insert(item.to.opposite(), stack);
		return stack.empty();
	}();

	if (!ok) {
		auto entPos = pos.as<float>().add(0.5, 0.5);
		auto vel = item.to.vec().as<float>() * 2;
		entPos -= item.to.vec().as<float>() * 0.2;
		ctx.plane.entities().spawn<ItemStackEntity>(entPos, vel, item.item);
	}
}

void ItemPipeTileEntity::onDespawn(const Swan::Context &ctx)
{
	auto pos = tileEntity_.pos.as<float>().add(0.5, 0.5);
	for (auto &item: content_) {
		ctx.plane.entities().spawn<ItemStackEntity>(pos, item.item);
	}

	if (inbox_.content_) {
		ctx.plane.entities().spawn<ItemStackEntity>(
			pos, inbox_.content_.value().item);
	}
}

void ItemPipeTileEntity::serialize(
	const Swan::Context &ctx, Proto::Builder w)
{
	tileEntity_.serialize(w.initTileEntity());

	if (inbox_.content_) {
		auto inboxW = w.initInbox();
		inbox_.content_->from.serialize(inboxW.initFrom());
		inboxW.setItem(inbox_.content_->item->name);
	}

	auto contentsW = w.initContents(content_.size());
	for (size_t i = 0; i < content_.size(); ++i) {
		contentsW[i].setItem(content_[i].item->name);
		content_[i].from.serialize(contentsW[i].initFrom());
		content_[i].to.serialize(contentsW[i].initTo());
		contentsW[i].setTimer(content_[i].timer);
	}
}

void ItemPipeTileEntity::deserialize(
	const Swan::Context &ctx, Proto::Reader r)
{
	tileEntity_.deserialize(r.getTileEntity());

	inbox_.content_.reset();
	if (r.hasInbox()) {
		inbox_.content_ = {};
		auto &inboxContents = *inbox_.content_;
		inboxContents.from.deserialize(r.getInbox().getFrom());
		inboxContents.item = &ctx.world.getItem(r.getInbox().getItem().cStr());
	}

	content_.clear();
	auto contentsR = r.getContents();
	content_.resize(contentsR.size());
	for (size_t i = 0; i < contentsR.size(); ++i) {
		content_[i].item = &ctx.world.getItem(contentsR[i].getItem().cStr());
		content_[i].from.deserialize(contentsR[i].getFrom());
		content_[i].to.deserialize(contentsR[i].getTo());
		content_[i].timer = contentsR[i].getTimer();
	}
}

Swan::ItemStack ItemPipeTileEntity::Inbox::insertInto(
	Swan::ItemStack stack, int from, int to)
{
	return insert(Swan::Direction::random(), stack);
}

Swan::ItemStack ItemPipeTileEntity::Inbox::insertSided(
	Swan::Direction from, Swan::ItemStack stack)
{
	if (content_) {
		return stack;
	}

	auto oneItem = stack.remove(1);
	if (oneItem.empty()) {
		return stack;
	}

	content_ = {oneItem.item(), from};
	return stack;
}

}
