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
	const Swan::Context &ctx, sbon::ObjectWriter w)
{
	tileEntity_.serialize(w.key("ent"));

	if (inbox_.contents_) {
		w.key("inbox").writeArray([&](sbon::Writer w) {
			w.writeString(inbox_.contents_->item->name);
			inbox_.contents_->from.serialize(w);
		});
	} else {
		w.key("inbox").writeNull();
	}

	w.key("contents").writeArray([&](sbon::Writer w) {
		for (auto &item: contents_) {
			w.writeArray([&](sbon::Writer w) {
				w.writeString(item.item->name);
				item.from.serialize(w);
				item.to.serialize(w);
				w.writeInt(item.timer);
			});
		}
	});
}

void ItemPipeTileEntity::deserialize(
	const Swan::Context &ctx, sbon::ObjectReader r)
{
	std::string str;
	r.match({
		{"ent", [&](sbon::Reader val) {
			tileEntity_.deserialize(val);
		}},
		{"inbox", [&](sbon::Reader val) {
			if (val.getType() == sbon::Type::NIL) {
				val.getNil();
				inbox_.contents_ = std::nullopt;
				return;
			}

			InboxItem item;

			val.getArray([&](sbon::ArrayReader val) {
				val.next().getString(str);
				item.item = &ctx.world.getItem(str);

				item.from.deserialize(val.next());

				inbox_.contents_ = item;
			});
		}},
		{"contents", [&](sbon::Reader val) {
			contents_.clear();
			val.readArray([&](sbon::Reader val) {
				val.getArray([&](sbon::ArrayReader val) {
					MovingItem item;

					val.next().getString(str);
					item.item = &ctx.world.getItem(str);
					item.from.deserialize(val.next());
					item.to.deserialize(val.next());
					item.timer = val.next().getInt();
					contents_.push_back(item);
				});
			});
		}},
	});
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
