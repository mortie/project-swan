#include <swan/swan.h>

#include "DefaultWorldGen.h"
#include "entities/DynamiteEntity.h"
#include "entities/PlayerEntity.h"
#include "entities/ItemStackEntity.h"
#include "entities/SpiderEntity.h"
#include "entities/FallingTileEntity.h"
#include "world/bonfire.h"
#include "world/chest.h"
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
		registerSprite("misc/burning-dynamite");
		registerSprite("ui/selected-slot");
		registerSprite("ui/inventory");

		registerSound("sounds/break/glass");
		registerSound("sounds/place/dirt");
		registerSound("sounds/place/leaves");
		registerStepSounds("sounds/step/glass");
		registerStepSounds("sounds/step/grass");
		registerStepSounds("sounds/step/metal");
		registerStepSounds("sounds/step/sand");
		registerStepSounds("sounds/step/stone");
		registerSound("sounds/misc/explosion");
		registerSound("sounds/misc/snap");
		registerSound("sounds/misc/splash");
		registerSound("sounds/misc/splash-short");
		registerSound("sounds/misc/teleport");
		registerSound("sounds/misc/lock-open");
		registerSound("sounds/misc/lock-close");
		registerSound("sounds/ui/crafting");
		registerSound("sounds/ui/inventory-open");
		registerSound("sounds/ui/inventory-close");

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
			.name = "leaves",
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
			.onBreak = dropRandomItemCount<"core::fiber">,
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
			.name = "boulder",
			.image = "core::tiles/geo/boulder",
			.isSolid = false,
			.breakableBy = Swan::Tool::HAND,
			.onBreak = dropRandomItemCount<"core::rock", 5>,
		});

		registerTile({
			.name = "water",
			.image = "@::invalid",
			.isSolid = false,
			.onSpawn = +[](const Swan::Context &ctx, Swan::TilePos pos) {
				ctx.plane.tiles().setIDWithoutUpdate(pos, Swan::World::AIR_TILE_ID);
				ctx.plane.fluids().setInTile(pos, ctx.world.getFluid("core::water").id);
				return true;
			},
		});
		registerTile({
			.name = "oil",
			.image = "@::invalid",
			.isSolid = false,
			.onSpawn = +[](const Swan::Context &ctx, Swan::TilePos pos) {
				ctx.plane.tiles().setIDWithoutUpdate(pos, Swan::World::AIR_TILE_ID);
				ctx.plane.fluids().setInTile(pos, ctx.world.getFluid("core::oil").id);
				return true;
			},
		});

		registerBonfire(*this);
		registerChest(*this);
		registerItemFan(*this);
		registerRopeLadder(*this);
		registerOutcrop(*this, "coal");
		registerOutcrop(*this, "iron", "iron-ore-chunk");
		registerOutcrop(*this, "copper", "copper-ore-chunk");
		registerOutcrop(*this, "sulphur");
		registerGlassPipe(*this);
		registerTorch(*this);

		registerItem({
			.name = "axe",
			.image = "core::tools/axe",
			.tool = Swan::Tool::AXE,
		});
		registerItem({
			.name = "dynamite",
			.image = "core::items/dynamite",
			.onActivate = [](
				const Swan::Context &ctx, Swan::ItemStack &stack,
				Swan::Vec2 pos, Swan::Vec2 dir)
			{
				stack.remove(1);
				ctx.plane.entities().spawn<DynamiteEntity>(pos, dir * 15);
			},
		});

		registerItem({
			.name = "rock",
			.image = "core::items/rock",
		});
		registerItem({
			.name = "coal",
			.image = "core::items/coal",
		});
		registerItem({
			.name = "pig-iron",
			.image = "core::items/pig-iron",
		});
		registerItem({
			.name = "iron-ore-chunk",
			.image = "core::items/iron-ore-chunk",
		});
		registerItem({
			.name = "copper",
			.image = "core::items/copper",
		});
		registerItem({
			.name = "copper-ore-chunk",
			.image = "core::items/copper-ore-chunk",
		});
		registerItem({
			.name = "sulphur",
			.image = "core::items/sulphur",
		});
		registerItem({
			.name = "fiber",
			.image = "core::items/fiber",
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
			.fg = {0.21, 0.68, 0.8, 0.8},
			.bg = {0.21, 0.68, 0.8, 0.8},
			.density = 1,
		});
		registerFluid({
			.name = "oil",
			.fg = {0.05, 0.02, 0.0, 0.95},
			.bg = {0.05, 0.02, 0.0, 1},
			.density = 1.5,
		});

		registerRecipeKind("crafting");
		registerRecipe({
			.inputs = {{1, "core::stick"}},
			.output = {1, "core::wood-pole"},
			.kind = "core::crafting",
		});
		registerRecipe({
			.inputs = {{1, "core::tree-trunk"}},
			.output = {8, "core::stick"},
			.kind = "core::crafting",
		});
		registerRecipe({
			.inputs = {{1, "core::stick"}, {1, "core::coal"}},
			.output = {2, "core::torch"},
			.kind = "core::crafting",
		});
		registerRecipe({
			.inputs = {{2, "core::fiber"}},
			.output = {1, "core::rope"},
			.kind = "core::crafting",
		});
		registerRecipe({
			.inputs = {{2, "core::rope"}, {2, "core::stick"}},
			.output = {1, "core::rope-ladder"},
			.kind = "core::crafting",
		});
		registerRecipe({
			.inputs = {
				{1, "core::rope"},
				{2, "core::stick"},
				{2, "core::rock"},
			},
			.output = {1, "core::axe"},
			.kind = "core::crafting",
		});
		registerRecipe({
			.inputs = {{1, "core::sulphur"}},
			.output = {1, "core::dynamite"},
			.kind = "core::crafting",
		});
		registerRecipe({
			.inputs = {
				{4, "core::tree-trunk"},
				{2, "core::rock"},
				{1, "core::rope"},
			},
			.output = {1, "core::chest"},
			.kind = "core::crafting",
		});
		registerRecipe({
			.inputs = {
				{3, "core::rock"},
				{4, "core::stick"},
				{2, "core::fiber"},
			},
			.output = {1, "core::bonfire"},
			.kind = "core::crafting",
		});

		registerRecipeKind("burning");
		registerRecipe({
			.inputs = {{1, "core::stick"}},
			.output = {},
			.kind = "core::burning",
		});
		registerRecipe({
			.inputs = {{1, "core::iron-ore-chunk"}},
			.output = {1, "core::pig-iron"},
			.kind = "core::burning",
		});
		registerRecipe({
			.inputs = {{1, "core::copper-ore-chunk"}},
			.output = {1, "core::copper"},
			.kind = "core::burning",
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
