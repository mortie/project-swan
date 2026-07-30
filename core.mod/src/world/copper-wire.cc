#include "copper-wire.h"
#include "entities/CopperWireEntity.h"
#include "entities/PlayerEntity.h"
#include "util/InteractionManager.h"
#include <memory>

namespace CoreMod {

class CopperWireInteractionManager: public InteractionManager {
public:
	CopperWireInteractionManager(Swan::EntityRef player, Swan::EntityRef wire):
		player_(player),
		wire_(wire)
	{}

	void update(Swan::Ctx &ctx, const Info &info) override
	{
		if (auto wire = wire_.as<CopperWireEntity>()) {
			wire->setEndPoint(info.lookPos);
		} else if (auto player = player_.as<PlayerEntity>()) {
			player->clearInteractionManager(ctx);
		}
	}

	void destroy(Swan::Ctx &ctx) override
	{
		ctx.plane.entities().despawn(wire_);
	}

private:
	Swan::EntityRef player_;
	Swan::EntityRef wire_;
};

static void onActivate(Swan::Ctx &ctx, Swan::Item::ActivateMeta meta)
{
	Swan::info << "Activate copper wire";
	auto player = meta.activator.as<PlayerEntity>();
	if (!player) {
		Swan::warn << "No player!!!";
		return;
	}

	Swan::info << "Spawning copper wire with start pos: " << Swan::tileCenter(meta.cursor);
	auto wire = ctx.plane.entities().spawn<CopperWireEntity>(
		Swan::tileCenter(meta.cursor));
	auto manager = std::make_unique<CopperWireInteractionManager>(meta.activator, wire);
	player->registerInteractionManager(ctx, std::move(manager));
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
