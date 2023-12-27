#include <swan/swan.h>

#include "DefaultWorldGen.h"
#include "entities/PlayerEntity.h"
#include "entities/ItemStackEntity.h"
#include "entities/SpiderEntity.h"
#include "world/tree.h"

namespace CoreMod {

class CoreMod: public Swan::Mod {
public:
	CoreMod(Swan::World &world): Swan::Mod("core") {
		breakListener_ = world.evtTileBreak_.subscribe(
			[&](auto ...args) { return onTileBreak(args...); });

		registerSprite("entity/player/idle");
		registerSprite("entity/player/running");
		registerSprite("entity/player/falling");
		registerSprite("entity/player/jumping");
		registerSprite("entity/player/landing");
		registerSprite("entity/spider/idle");
		registerSprite("misc/background-cave");

		registerTile({
			.name = "stone",
			.image = "core::tile/stone",
			.droppedItem = "core::stone",
		});
		registerTile({
			.name = "dirt",
			.image = "core::tile/dirt",
			.droppedItem = "core::dirt",
		});
		registerTile({
			.name = "grass",
			.image = "core::tile/grass",
			.droppedItem = "core::dirt",
		});
		registerTile({
			.name = "tree-trunk",
			.image = "core::tile/tree-trunk",
			.isSolid = false,
			.droppedItem = "core::tree-trunk",
			.onBreak = &breakTree,
			.traits = std::make_shared<TreeTrait>(),
		});
		registerTile({
			.name = "tree-leaves",
			.image = "core::tile/leaves",
			.isSolid = false,
			.onBreak = &breakTree,
			.traits = std::make_shared<TreeTrait>(),
		});
		registerTile({
			.name = "tree-seeder",
			.image = "core::tile/leaves",
			.onSpawn = spawnTree,
		});
		registerTile({
			.name = "torch",
			.image = "core::tile/torch",
			.isSolid = false,
			.lightLevel = 80/255.0,
			.droppedItem = "core::torch",
		});

		registerItem({
			.name = "wood-pole",
			.image = "core::tile/wood-pole",
		});

		registerRecipe({
			.inputs = {{1, "core::tree-trunk"}},
			.output = {8, "core::wood-pole"},
			.kind = "crafting",
		});
		registerRecipe({
			.inputs = {{1, "core::wood-pole"}},
			.output = {1, "core::torch"},
			.kind = "crafting",
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

}

extern "C" Swan::Mod *mod_create(Swan::World &world) {
	return new CoreMod::CoreMod(world);
}
