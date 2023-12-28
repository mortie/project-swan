#include <swan/swan.h>

#include "DefaultWorldGen.h"
#include "entities/PlayerEntity.h"
#include "entities/ItemStackEntity.h"
#include "entities/SpiderEntity.h"
#include "world/tree.h"
#include "world/util.h"

namespace CoreMod {

class CoreMod: public Swan::Mod {
public:
	CoreMod(Swan::World &world): Swan::Mod("core")
	{
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
			.lightLevel = 80 / 255.0,
			.droppedItem = "core::torch",
		});
		registerTile({
			.name = "tall-grass",
			.image = "core::tile/tall-grass",
			.isSolid = false,
			.onBreak = +[](const Swan::Context &ctx, Swan::TilePos pos) {
				if (Swan::randfloat() > 0.5) {
					dropItem(ctx, pos, "core::straw");
				}
			},
		});

		registerItem({
			.name = "wood-pole",
			.image = "core::item/wood-pole",
		});
		registerItem({
			.name = "straw",
			.image = "core::item/straw",
		});
		registerItem({
			.name = "rope",
			.image = "core::item/rope",
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
		registerRecipe({
			.inputs = {{2, "core::straw"}},
			.output = {8, "core::rope"},
			.kind = "crafting",
		});

		registerWorldGen<DefaultWorldGen>("default");

		registerEntity<PlayerEntity>("player");
		registerEntity<ItemStackEntity>("item-stack");
		registerEntity<SpiderEntity>("spider");
	}

	Swan::EventListener breakListener_;
};

}

extern "C" Swan::Mod *mod_create(Swan::World &world)
{
	return new CoreMod::CoreMod(world);
}
