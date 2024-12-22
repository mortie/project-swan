#include <swan/swan.h>

#include "DefaultWorldGen.h"
#include "entities/DynamiteEntity.h"
#include "entities/PlayerEntity.h"
#include "entities/ItemStackEntity.h"
#include "entities/SpiderEntity.h"
#include "entities/FallingTileEntity.h"
#include "world/item-fan.h"
#include "world/ladder.h"
#include "world/pipe.h"
#include "world/torch.h"
#include "world/tree.h"
#include "world/util.h"

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
		registerSprite("ui/selected-slot");
		registerSprite("ui/hotbar");

		registerSound("sounds/step/grass1");
		registerSound("sounds/step/grass2");
		registerSound("sounds/step/stone1");
		registerSound("sounds/step/stone2");
		registerSound("sounds/step/sand1");
		registerSound("sounds/step/sand2");
		registerSound("sounds/break/dirt");
		registerSound("sounds/break/leaves");
		registerSound("sounds/snap");

		registerTile({
			.name = "stone",
			.image = "core::tiles/stone",
			.stepSound = "core::sounds/step/stone",
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
			.stepSound = "core::sounds/step/sand",
			.droppedItem = "core::sand",
			.onTileUpdate = fallIfFloating,
		});
		registerTile({
			.name = "glass",
			.image = "core::tiles/glass",
			.isOpaque = false,
			.stepSound = "core::sounds/step/sand",
		});
		registerTile({
			.name = "grass",
			.image = "core::tiles/grass",
			.stepSound = "core::sounds/step/grass",
			.breakSound = "core::sounds/break/dirt",
			.droppedItem = "core::dirt",
		});
		registerTile({
			.name = "wood-pole",
			.image = "core::tiles/wood-pole",
			.isSolid = false,
			.isSupportV = true,
			.droppedItem = "core::wood-pole",
			.onSpawn = denyIfFloating,
			.onTileUpdate = breakIfFloating,
		});
		registerTile({
			.name = "tree-trunk",
			.image = "core::tiles/tree-trunk",
			.isSolid = false,
			.isSupportV = true,
			.isSupportH = true,
			.droppedItem = "core::tree-trunk",
			.onSpawn = denyIfFloating,
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
			.name = "tall-grass",
			.image = "core::tiles/tall-grass",
			.isSolid = false,
			.isReplacable = true,
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

		registerTile({
			.name = "water",
			.image = "@::invalid",
			.isSolid = false,
			.onSpawn = +[] (const Swan::Context &ctx, Swan::TilePos pos) {
				ctx.plane.setTileIDWithoutUpdate(pos, Swan::World::AIR_TILE_ID);
				ctx.plane.setFluid(pos, ctx.world.getFluid("core::water").id);
				return true;
			},
		});

		registerItemFan(*this);
		registerRopeLadder(*this);
		registerGlassPipe(*this);
		registerTorch(*this);

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
				ctx.plane.entities().spawn<DynamiteEntity>(pos, dir * 15);
			},
		});

		registerFluid({
			.name = "water",
			.color = {0.21, 0.78, 0.78, 0.9},
		});

		registerRecipe({
			.inputs = {{1, "core::tree-trunk"}},
			.output = {8, "core::wood-pole"},
			.kind = "crafting",
		});
		registerRecipe({
			.inputs = {{4, "core::wood-pole"}},
			.output = {4, "core::torch"},
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
