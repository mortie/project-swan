#include <swan/swan.h>

#include "DefaultWorldGen.h"
#include "entities/DynamiteEntity.h"
#include "entities/PlayerEntity.h"
#include "entities/ItemStackEntity.h"
#include "entities/SpiderEntity.h"
#include "entities/FallingTileEntity.h"
#include "world/item-fan.h"
#include "world/ladder.h"
#include "world/outcrop.h"
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

		registerSound("sounds/break/glass");
		registerSound("sounds/place/dirt");
		registerSound("sounds/place/leaves");
		registerStepSounds("sounds/step/glass");
		registerStepSounds("sounds/step/grass");
		registerStepSounds("sounds/step/metal");
		registerStepSounds("sounds/step/sand");
		registerStepSounds("sounds/step/stone");
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
			.placeSound = "core::sounds/place/dirt",
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
			.breakableBy = Swan::Tool::HAND,
			.stepSound = "core::sounds/step/glass",
			.breakSound = "core::sounds/break/glass",
		});
		registerTile({
			.name = "grass",
			.image = "core::tiles/grass",
			.stepSound = "core::sounds/step/grass",
			.placeSound = "core::sounds/place/dirt",
			.droppedItem = "core::dirt",
		});
		registerTile({
			.name = "wood-pole",
			.image = "core::tiles/wood-pole",
			.isSolid = false,
			.isSupportV = true,
			.breakableBy = Swan::Tool::HAND,
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
			.breakableBy = Swan::Tool::AXE,
			.droppedItem = "core::tree-trunk",
			.onSpawn = denyIfFloating,
			.onTileUpdate = breakIfFloating,
			.traits = std::make_shared<TreeTrunkTrait>(),
		});
		registerTile({
			.name = "tree-leaves",
			.image = "core::tiles/leaves",
			.isSolid = false,
			.breakableBy = Swan::Tool::HAND,
			.stepSound = "core::sounds/step/grass",
			.placeSound = "core::sounds/place/leaves",
			.onBreak = dropRandomItemCount<"core::stick">,
			.onTileUpdate = breakTreeLeavesIfFloating,
			.traits = std::make_shared<TreeLeavesTrait>(),
		});

		registerTile({
			.name = "tall-grass",
			.image = "core::tiles/flora/tall-grass",
			.isSolid = false,
			.isReplacable = true,
			.breakableBy = Swan::Tool::HAND,
			.placeSound = "core::sounds/place/leaves",
			.onBreak = dropRandomItemCount<"core::straw">,
			.onTileUpdate = breakIfFloating,
		});
		registerTile({
			.name = "dead-shrub",
			.image = "core::tiles/flora/dead-shrub",
			.isSolid = false,
			.breakableBy = Swan::Tool::HAND,
			.placeSound = "core::sounds/place/leaves",
			.onBreak = dropRandomItemCount<"core::stick">,
			.onTileUpdate = breakIfFloating,
		});

		registerTile({
			.name = "water",
			.image = "@::invalid",
			.isSolid = false,
			.onSpawn = +[] (const Swan::Context &ctx, Swan::TilePos pos) {
				ctx.plane.tiles().setIDWithoutUpdate(pos, Swan::World::AIR_TILE_ID);
				ctx.plane.fluids().setInTile(pos, ctx.world.getFluid("core::water").id);
				return true;
			},
		});

		registerItemFan(*this);
		registerRopeLadder(*this);
		registerOutcrop(*this, "coal");
		registerGlassPipe(*this);
		registerTorch(*this);

		registerItem({
			.name = "axe",
			.image = "core::items/axe",
			.tool = Swan::Tool::AXE,
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

		registerItem({
			.name = "coal",
			.image = "core::items/coal",
		});
		registerItem({
			.name = "straw",
			.image = "core::items/straw",
		});
		registerItem({
			.name = "stick",
			.image = "core::items/stick",
		});
		registerItem({
			.name = "rope",
			.image = "core::items/rope",
		});

		registerFluid({
			.name = "water",
			.color = {0.21, 0.78, 0.9, 0.7},
			.density = 1,
		});
		registerFluid({
			.name = "oil",
			.color = {0.05, 0.02, 0.0, 0.98},
			.density = 1.5,
		});

		registerRecipe({
			.inputs = {{1, "core::stick"}},
			.output = {1, "core::wood-pole"},
			.kind = "crafting",
		});
		registerRecipe({
			.inputs = {{1, "core::tree-trunk"}},
			.output = {8, "core::stick"},
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
			.inputs = {{2, "core::rope"}, {2, "core::stick"}},
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
