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

	auto &ents = ctx.plane.getEntitiesInArea(pos, size);
	auto force = dir_.vec().as<float>() * 500;
	for (auto &found: ents) {
		found.ref.traitThen<Swan::PhysicsBodyTrait>([&](auto &physics) {
			physics.applyForce(force);
		});
	}
}

void ItemFanTileEntity::tick(const Swan::Context &ctx, float dt)
{
	Swan::Vec2 pos;
	Swan::Vec2 size = {0.1, 1};
	if (dir_ == Swan::Direction::LEFT) {
		pos = tileEntity_.pos.as<float>().add(1, 0);
	} else {
		pos = tileEntity_.pos.as<float>().add(-size.x, 0);
	}

	auto &ents = ctx.plane.getEntitiesInArea(pos, size);
	for (auto &found: ents) {
		auto *stackEnt = dynamic_cast<ItemStackEntity *>(found.ref.get());
		if (!stackEnt) {
			continue;
		}

		auto inv = ctx.plane.getTileEntity(tileEntity_.pos + dir_)
			.trait<Swan::InventoryTrait>();
		if (!inv) {
			return;
		}

		Swan::ItemStack stack(stackEnt->item(), 1);
		stack = inv->insert(dir_.opposite(), stack);

		if (stack.empty()) {
			ctx.plane.despawnEntity(found.ref);
		}

		return;
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
