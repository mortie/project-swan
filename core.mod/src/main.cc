#include <swan/swan.h>

#include "DefaultWorldGen.h"
#include "entities/PlayerEntity.h"
#include "entities/ItemStackEntity.h"
#include "entities/SpiderEntity.h"
#include "world/tree.h"

#include <functional>

class CoreMod: public Swan::Mod {
public:
	CoreMod(Swan::World &world): Swan::Mod("core") {
		breakListener_ = world.evtTileBreak_.subscribe(
			[&](auto ...args) { return onTileBreak(args...); });

		registerSprite("entity/player-running");
		registerSprite("entity/player-idle");
		registerSprite("entity/spider-idle");
		registerSprite("misc/background-cave");

		registerTileWithItem({
			.name = "stone",
			.image = "core::tile/stone",
			.droppedItem = "core::stone",
		});
		registerTileWithItem({
			.name = "dirt",
			.image = "core::tile/dirt",
			.droppedItem = "core::dirt",
		});
		registerTileWithItem({
			.name = "grass",
			.image = "core::tile/grass",
			.droppedItem = "core::dirt",
		});
		registerTileWithItem({
			.name = "tree-trunk",
			.image = "core::tile/tree-trunk",
			.isSolid = false,
			.droppedItem = "core::tree-trunk",
		});
		registerTileWithItem({
			.name = "tree-leaves",
			.image = "core::tile/leaves",
			.isSolid = false,
		});
		registerTileWithItem({
			.name = "tree-seeder",
			.image = "core::tile/leaves",
			.onSpawn = spawnTree,
		});
		registerTileWithItem({
			.name = "torch",
			.image = "core::tile/torch",
			.isSolid = false,
			.lightLevel = 80/255.0,
		});

		registerWorldGen<DefaultWorldGen>("default");

		registerEntity<PlayerEntity>("player");
		registerEntity<ItemStackEntity>("item-stack");
		registerEntity<SpiderEntity>("spider");
	}

	void onTileBreak(const Swan::Context &ctx, Swan::TilePos pos, Swan::Tile &tile) {
		if (tile.droppedItem) {
			ctx.plane.spawnEntity<ItemStackEntity>(
				(Swan::Vec2)pos + Swan::Vec2{0.5, 0.5}, *tile.droppedItem);
		}
	}

	Swan::EventListener breakListener_;
};

extern "C" Swan::Mod *mod_create(Swan::World &world) {
	return new CoreMod(world);
}
