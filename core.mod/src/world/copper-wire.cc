#include "copper-wire.h"
#include "entities/CopperWireEntity.h"
#include "entities/PlayerEntity.h"
#include "swan/EntityCollection.h"
#include "swan/traits/InventoryTrait.h"
#include "traits/PowerBufferTrait.h"
#include "traits/PowerNodeTrait.h"
#include "util/InteractionManager.h"
#include <memory>

namespace CoreMod {

class CopperWireInteractionManager: public InteractionManager {
public:
	CopperWireInteractionManager(
		Swan::Ctx &ctx,
		Swan::EntityRef player,
		Swan::EntityRef startEntity,
		Swan::Vec2 startPoint
	):
		player_(player),
		startEntity_(startEntity),
		wire_(ctx, startPoint)
	{
		Swan::info << "Copper wire interaction manager, player: " << player << ", start entity: " << startEntity;
		wire_.begin_ = startEntity;
	}

	int playerCopperSlot()
	{
		auto inventory = player_.trait<Swan::InventoryTrait>();
		auto content = inventory->content();
		for (int i = 0; i < content.size(); ++i) {
			auto &stack = content[i];
			if (stack.empty()) {
				continue;
			}

			if (stack.item()->name == "core::copper-wire") {
				return i;
			}
		}

		return -1;
	}

	void update(Swan::Ctx &ctx, float dt, const Info &info) override
	{
		if (!startEntity_ || playerCopperSlot() < 0) {
			player_.as<PlayerEntity>()->clearInteractionManager();
			return;
		}

		wire_.setEndPoint(info.lookPos);
		wire_.update(ctx, dt);
	}

	bool activate(Swan::Ctx &ctx, const Info &info) override
	{
		auto ent = ctx.plane.entities().getTileEntity(info.placePos);
		auto startPowerNode = startEntity_.trait<PowerNodeTrait>();
		auto endPowerNode = ent.trait<PowerNodeTrait>();
		int copperSlot = playerCopperSlot();

		bool ok = (
			startPowerNode &&
			endPowerNode &&
			copperSlot >= 0);
		if (!ok) {
			return false;
		}

		wire_.end_ = ent;

		// Consume a piece of copper
		auto player = player_.as<PlayerEntity>();
		auto &inventory = player->get(Swan::InventoryTrait::Tag{});
		auto stack = inventory.take(copperSlot);
		stack.remove(1);
		inventory.set(copperSlot, stack);

		// Set end point
		auto endPoint = info.placePos.as<float>() + endPowerNode->anchorPoint();
		wire_.setEndPoint(endPoint);

		// Spawn the wire entity properly, then attach it to the power nodes
		auto ref = ctx.plane.entities().spawnMove(std::move(wire_));
		startPowerNode->attach(ref);
		endPowerNode->attach(ref);
		endPowerNode->invalidateNetwork();

		player_.as<PlayerEntity>()->clearInteractionManager();
		return true;
	}

	void draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd) override
	{
		wire_.draw(ctx, rnd);
	}

private:
	Swan::EntityRef player_;
	Swan::EntityRef startEntity_;
	CopperWireEntity wire_;
};

static bool onActivate(Swan::Ctx &ctx, Swan::Item::ActivateMeta meta)
{
	auto player = meta.activator.as<PlayerEntity>();
	if (!player) {
		return false;
	}

	auto ent = ctx.plane.entities().getTileEntity(meta.cursor);
	auto powerNode = ent.trait<PowerNodeTrait>();
	if (!powerNode) {
		return false;
	}

	auto startPoint = meta.cursor.as<float>() + powerNode->anchorPoint();
	auto manager = std::make_unique<CopperWireInteractionManager>(
		ctx, meta.activator, ent, startPoint);
	player->registerInteractionManager(std::move(manager));
	return true;
}

void registerCopperWire(Swan::Mod &mod)
{
	mod.registerEntity<CopperWireEntity>("copper-wire");

	mod.registerItem({
		.name = "copper-wire",
		.image = "core::items/copper-wire",
		.onActivate = onActivate,
	});
}

}
