#include "ItemFanTileEntity.h"

#include "entities/ItemStackEntity.h"

namespace CoreMod {

void ItemFanTileEntity::update(const Swan::Context &ctx, float dt)
{
	Swan::Vec2 pos;
	Swan::Vec2 size = {2, 1};
	if (dir_ == Swan::Direction::LEFT) {
		pos = tileEntity_.pos.as<float>().add(1, 0);
	} else {
		pos = tileEntity_.pos.as<float>().add(-size.x, 0);
	}

	Swan::Vec2 center = tileEntity_.pos.as<float>().add(0.5, 0.5);

	auto &ents = ctx.plane.getEntitiesInArea(pos, size);
	auto force = dir_.vec().as<float>() * 500;
	for (auto &found: ents) {
		if (
				!pickup_ &&
				(found.body.center() - center).squareLength() < 0.9 * 0.9 &&
				found.ref.as<ItemStackEntity>()) {
			pickup_ = found.ref;
		}

		found.ref.traitThen<Swan::PhysicsBodyTrait>([&](auto &physics) {
			physics.applyForce(force);
		});
	}
}

void ItemFanTileEntity::tick(const Swan::Context &ctx, float dt)
{
	auto pickup = pickup_;
	pickup_ = {};
	if (!pickup) {
		return;
	}

	auto inv = ctx.plane.getTileEntity(tileEntity_.pos + dir_)
		.trait<Swan::InventoryTrait>();
	if (!inv) {
		return;
	}

	auto *stackEnt = pickup.as<ItemStackEntity>();
	if (!stackEnt) {
		return;
	}

	Swan::ItemStack stack(stackEnt->item(), 1);
	stack = inv->insert(dir_.opposite(), stack);

	if (stack.empty()) {
		ctx.plane.despawnEntity(pickup);
	}
}

void ItemFanTileEntity::serialize(const Swan::Context &ctx, sbon::ObjectWriter w)
{
	dir_.serialize(w.key("dir"));
	tileEntity_.serialize(w.key("ent"));
}

void ItemFanTileEntity::deserialize(const Swan::Context &ctx, sbon::ObjectReader r)
{
	r.match({
		{"dir", [&](sbon::Reader val) {
			dir_.deserialize(val);
		}},
		{"ent", [&](sbon::Reader val) {
			tileEntity_.deserialize(val);
		}},
	});
}

}
