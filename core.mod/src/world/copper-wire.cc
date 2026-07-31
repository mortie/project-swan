#include "copper-wire.h"
#include "entities/CopperWireEntity.h"
#include "entities/PlayerEntity.h"
#include "util/InteractionManager.h"
#include <memory>

namespace CoreMod {

class CopperWireInteractionManager: public InteractionManager {
public:
	CopperWireInteractionManager(Swan::Ctx &ctx, Swan::EntityRef player, Swan::Vec2 startPoint):
		player_(player),
		wire_(ctx, startPoint)
	{}

	void update(Swan::Ctx &ctx, float dt, const Info &info) override
	{
		wire_.setEndPoint(info.lookPos);
		wire_.update(ctx, dt);
	}

	void activate(Swan::Ctx &ctx, const Info &info) override
	{
		// TODO: do something more appropriate here
		player_.as<PlayerEntity>()->clearInteractionManager();
	}

	void draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd) override
	{
		wire_.draw(ctx, rnd);
	}

private:
	Swan::EntityRef player_;
	CopperWireEntity wire_;
};

static void onActivate(Swan::Ctx &ctx, Swan::Item::ActivateMeta meta)
{
	Swan::info << "Activate copper wire";
	auto player = meta.activator.as<PlayerEntity>();
	if (!player) {
		Swan::warn << "No player!!!";
		return;
	}

	auto manager = std::make_unique<CopperWireInteractionManager>(
		ctx, meta.activator, Swan::tileCenter(meta.cursor));
	player->registerInteractionManager(std::move(manager));
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
