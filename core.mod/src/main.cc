#include <swan/swan.h>

#include "DefaultWorldGen.h"
#include "entities/DynamiteEntity.h"
#include "entities/PlayerEntity.h"
#include "entities/ItemStackEntity.h"
#include "entities/SpiderEntity.h"
#include "entities/FallingTileEntity.h"
#include "world/ladder.h"
#include "world/pipe.h"
#include "world/tree.h"
#include "world/util.h"

#include <thread>
#include <atomic>
#include <chrono>

namespace CoreMod {

class CoreMod: public Swan::Mod {
public:
	CoreMod(): Swan::Mod("core")
	{
		registerSprite("entities/player/idle");
		registerSprite("entities/player/running");
		registerSprite("entities/player/falling");
		registerSprite("entities/player/jumping");
		registerSprite("entities/player/landing");
		registerSprite("entities/spider/idle");
		registerSprite("misc/background-cave");

		registerSound("sounds/step/grass1");
		registerSound("sounds/step/grass2");
		registerSound("sounds/break/dirt");
		registerSound("sounds/break/leaves");
		registerSound("sounds/snap");

		registerTile({
			.name = "stone",
			.image = "core::tiles/stone",
			.droppedItem = "core::stone",
		});
		registerTile({
			.name = "dirt",
			.image = "core::tiles/dirt",
			.stepSound = "core::sounds/step/grass",
			.breakSound = "core::sounds/break/dirt",
			.droppedItem = "core::dirt",
		});
		registerTile({
			.name = "sand",
			.image = "core::tiles/sand",
			.droppedItem = "core::sand",
			.onTileUpdate = fallIfFloating,
		});
		registerTile({
			.name = "grass",
			.stepSound = "core::sounds/step/grass",
			.image = "core::tiles/grass",
			.breakSound = "core::sounds/break/dirt",
			.droppedItem = "core::dirt",
		});
		registerTile({
			.name = "tree-trunk",
			.image = "core::tiles/tree-trunk",
			.isSolid = false,
			.isOpaque = true,
			.droppedItem = "core::tree-trunk",
			.onTileUpdate = breakIfFloating,
			.traits = std::make_shared<TreeTrunkTrait>(),
		});
		registerTile({
			.name = "tree-leaves",
			.image = "core::tiles/leaves",
			.isSolid = false,
			.stepSound = "core::sounds/step/grass",
			.breakSound = "core::sounds/break/leaves",
			.onTileUpdate = breakTreeLeavesIfFloating,
			.traits = std::make_shared<TreeLeavesTrait>(),
		});
		registerTile({
			.name = "tree-seeder",
			.image = "core::tiles/leaves",
			.onSpawn = spawnTree,
		});
		registerTile({
			.name = "torch",
			.image = "core::tiles/torch",
			.isSolid = false,
			.lightLevel = 80 / 255.0,
			.droppedItem = "core::torch",
		});
		registerTile({
			.name = "tall-grass",
			.image = "core::tiles/tall-grass",
			.isSolid = false,
			.breakSound = "core::sounds/break/leaves",
			.onBreak = +[] (const Swan::Context &ctx, Swan::TilePos pos) {
				for (int i = 0; i < 3; ++i) {
					if (Swan::randfloat() > 0.25) {
						dropItem(ctx, pos, "core::straw");
					}
				}
			},
			.onTileUpdate = breakIfFloating,
		});

		registerRopeLadder(*this);
		registerGlassPipe(*this);

		registerItem({
			.name = "wood-pole",
			.image = "core::items/wood-pole",
		});
		registerItem({
			.name = "straw",
			.image = "core::items/straw",
		});
		registerItem({
			.name = "rope",
			.image = "core::items/rope",
		});
		registerItem({
			.name = "dynamite",
			.image = "core::items/dynamite",
			.onActivate = [](
				const Swan::Context &ctx, Swan::InventorySlot slot,
				Swan::Vec2 pos, Swan::Vec2 dir)
			{
				slot.remove(1);
				ctx.plane.spawnEntity<DynamiteEntity>(pos, dir * 10);
			},
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
			.output = {1, "core::rope"},
			.kind = "crafting",
		});
		registerRecipe({
			.inputs = {{2, "core::rope"}, {2, "core::wood-pole"}},
			.output = {1, "core::rope-ladder"},
			.kind = "crafting",
		});

		registerWorldGen<DefaultWorldGen>("default");

		registerEntity<PlayerEntity>("player");
		registerEntity<ItemStackEntity>("item-stack");
		registerEntity<SpiderEntity>("spider");
		registerEntity<FallingTileEntity>("falling-tile");
		registerEntity<DynamiteEntity>("dynamite");
	}
};

}

extern "C" Swan::Mod *mod_create()
{
	return new CoreMod::CoreMod();
}
