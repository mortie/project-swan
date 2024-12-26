#include "ItemPipeTileEntity.h"

#include <optional>

#include "entities/ItemStackEntity.h"

namespace CoreMod {

void ItemPipeTileEntity::draw(const Swan::Context &ctx, Cygnet::Renderer &rnd)
{
	Swan::Vec2 center = tileEntity_.pos.as<float>().add(0.5, 0.5);

	for (auto &item: contents_) {
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

void ItemPipeTileEntity::tick2(const Swan::Context &ctx, float dt)
{
	if (inbox_.contents_ && contents_.size() < 10) {
		auto input = inbox_.contents_.value();
		inbox_.contents_.reset();

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
	for (auto &item: contents_) {
		ctx.plane.entities().spawn<ItemStackEntity>(pos, item.item);
	}

	if (inbox_.contents_) {
		ctx.plane.entities().spawn<ItemStackEntity>(
			pos, inbox_.contents_.value().item);
	}
}

void ItemPipeTileEntity::serialize(
	const Swan::Context &ctx, Proto::Builder w)
{
	tileEntity_.serialize(w.initTileEntity());

	if (inbox_.contents_) {
		auto inboxW = w.initInbox();
		inbox_.contents_->from.serialize(inboxW.initFrom());
		inboxW.setItem(inbox_.contents_->item->name);
	}

	auto contentsW = w.initContents(contents_.size());
	for (size_t i = 0; i < contents_.size(); ++i) {
		contentsW[i].setItem(contents_[i].item->name);
		contents_[i].from.serialize(contentsW[i].initFrom());
		contents_[i].to.serialize(contentsW[i].initTo());
		contentsW[i].setTimer(contents_[i].timer);
	}
}

void ItemPipeTileEntity::deserialize(
	const Swan::Context &ctx, Proto::Reader r)
{
	tileEntity_.deserialize(r.getTileEntity());

	inbox_.contents_.reset();
	if (r.hasInbox()) {
		inbox_.contents_ = {};
		auto &inboxContents = *inbox_.contents_;
		inboxContents.from.deserialize(r.getInbox().getFrom());
		inboxContents.item = &ctx.world.getItem(r.getInbox().getItem().cStr());
	}

	contents_.clear();
	auto contentsR = r.getContents();
	contents_.resize(contentsR.size());
	for (size_t i = 0; i < contentsR.size(); ++i) {
		contents_[i].item = &ctx.world.getItem(contentsR[i].getItem().cStr());
		contents_[i].from.deserialize(contentsR[i].getFrom());
		contents_[i].to.deserialize(contentsR[i].getTo());
		contents_[i].timer = contentsR[i].getTimer();
	}
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
