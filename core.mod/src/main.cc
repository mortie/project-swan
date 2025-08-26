#include <swan/swan.h>

#include "DefaultWorldGen.h"
#include "entities/DynamiteEntity.h"
#include "entities/PlayerEntity.h"
#include "entities/ItemStackEntity.h"
#include "entities/SpiderEntity.h"
#include "entities/FallingTileEntity.h"
#include "world/aqueduct.h"
#include "world/bonfire.h"
#include "world/chest.h"
#include "world/clay.h"
#include "world/crucible.h"
#include "world/drain.h"
#include "world/item-fan.h"
#include "world/ladder.h"
#include "world/outcrop.h"
#include "world/pipe.h"
#include "world/potato.h"
#include "world/torch.h"
#include "world/tree.h"
#include "world/util.h"

namespace CoreMod {

class CoreMod: public Swan::Mod {
public:
	CoreMod(): Swan::Mod("core")
	{
		registerTile({
			.name = "stone",
			.image = "core::tiles/stone",
			.stepSound = "core::step/stone",
			.droppedItem = "core::stone",
		});
		registerTile({
			.name = "dirt",
			.image = "core::tiles/dirt",
			.stepSound = "core::step/grass",
			.placeSound = "core::place/dirt",
			.droppedItem = "core::dirt",
			.onWorldTick = +[](Swan::Ctx &ctx, Swan::TilePos pos) {
				if (ctx.plane.tiles().get(pos.add(0, -1)).isSolid()) {
					return;
				}

				auto isGrass = +[](Swan::Ctx &ctx, Swan::TilePos pos) {
					return ctx.plane.tiles().get(pos).name == "core::grass";
				};
				bool hasNearbyGrass =
					isGrass(ctx, pos.add(-1, 0)) ||
					isGrass(ctx, pos.add(1, 0)) ||
					isGrass(ctx, pos.add(-1, -1)) ||
					isGrass(ctx, pos.add(1, -1)) ||
					isGrass(ctx, pos.add(-1, 1)) ||
					isGrass(ctx, pos.add(1, 1));
				if (!hasNearbyGrass) {
					return;
				}

				auto fluid = ctx.plane.fluids().getAtPos(pos.as<float>().add(0.5, -0.2));
				if (fluid.density > 0) {
					return;
				}

				ctx.plane.tiles().set(pos, "core::grass");
			},
		});
		registerTile({
			.name = "sand",
			.image = "core::tiles/sand",
			.stepSound = "core::step/sand",
			.droppedItem = "core::sand",
			.onTileUpdate = fallIfFloating,
		});
		registerTile({
			.name = "glass",
			.image = "core::tiles/glass",
			.isOpaque = false,
			.breakableBy = Swan::Tool::HAND,
			.stepSound = "core::step/glass",
			.breakSound = "core::break/glass",
		});
		registerTile({
			.name = "grass",
			.image = "core::tiles/grass",
			.stepSound = "core::step/grass",
			.placeSound = "core::place/dirt",
			.droppedItem = "core::dirt",
			.onTileUpdate = +[](Swan::Ctx &ctx, Swan::TilePos pos) {
				if (ctx.plane.tiles().get(pos.add(0, -1)).isOpaque()) {
					ctx.plane.tiles().set(pos, "core::dirt");
				}
			},
			.onWorldTick = +[](Swan::Ctx &ctx, Swan::TilePos pos) {
				auto fluid = ctx.plane.fluids().getAtPos(pos.as<float>().add(0.5, -0.2));
				if (fluid.density > 0) {
					ctx.plane.tiles().set(pos, "core::dirt");
				}
			},
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
			.name = "tall-grass",
			.image = "core::tiles/flora/tall-grass",
			.isSolid = false,
			.isReplacable = true,
			.breakableBy = Swan::Tool::HAND,
			.placeSound = "core::place/leaves",
			.onBreak = dropRandomItemCount<"core::fiber">,
			.onTileUpdate = breakIfFloating,
		});
		registerTile({
			.name = "dead-shrub",
			.image = "core::tiles/flora/dead-shrub",
			.isSolid = false,
			.breakableBy = Swan::Tool::HAND,
			.placeSound = "core::place/leaves",
			.onBreak = dropRandomItemCount<"core::stick">,
			.onTileUpdate = breakIfFloating,
		});
		registerTile({
			.name = "boulder",
			.image = "core::tiles/geo/boulder",
			.isSolid = false,
			.breakableBy = Swan::Tool::HAND,
			.onBreak = dropRandomItemCount<"core::rock", 5>,
			.onTileUpdate = breakIfFloating,
		});

		registerTile({
			.name = "water",
			.image = "@::invalid",
			.isSolid = false,
			.onSpawn = +[](Swan::Ctx &ctx, Swan::TilePos pos) {
				ctx.plane.tiles().setIDWithoutUpdate(pos, Swan::World::AIR_TILE_ID);
				ctx.plane.fluids().setInTile(pos, ctx.world.getFluid("core::water").id);
				return true;
			},
		});
		registerTile({
			.name = "oil",
			.image = "@::invalid",
			.isSolid = false,
			.onSpawn = +[](Swan::Ctx &ctx, Swan::TilePos pos) {
				ctx.plane.tiles().setIDWithoutUpdate(pos, Swan::World::AIR_TILE_ID);
				ctx.plane.fluids().setInTile(pos, ctx.world.getFluid("core::oil").id);
				return true;
			},
		});

		registerAqueduct(*this);
		registerBonfire(*this);
		registerChest(*this);
		registerClay(*this);
		registerCrucible(*this);
		registerDrain(*this);
		registerItemFan(*this);
		registerRopeLadder(*this);
		registerOutcrop(*this, "coal");
		registerOutcrop(*this, "iron", "iron-ore-chunk");
		registerOutcrop(*this, "copper", "copper-ore-chunk");
		registerOutcrop(*this, "sulphur");
		registerGlassPipe(*this);
		registerPotato(*this);
		registerTorch(*this);
		registerTree(*this);

		registerItem({
			.name = "axe",
			.image = "core::items/tools/axe",
			.tool = Swan::Tool::AXE,
		});
		registerItem({
			.name = "dynamite",
			.image = "core::items/dynamite",
			.onActivate = [](Swan::Ctx &ctx, Swan::Item::ActivateMeta meta) {
				meta.stack.remove(1);
				auto pos = meta.activator.getBody()->topMid();
				auto vel = meta.direction * 15;
				ctx.plane.entities().spawn<DynamiteEntity>(pos, vel);
			},
		});

		registerItem({
			.name = "rock",
			.image = "core::items/rock",
		});
		registerItem({
			.name = "clay",
			.image = "core::items/clay",
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
		registerItem({
			.name = "unfired-crucible",
			.image = "core::items/unfired-crucible",
		});

		registerItem({
			.name = "burned-food",
			.image = "core::items/burned-food",
		});
		registerItem({
			.name = "potato",
			.image = "core::items/potato",
			.onActivate = +[](Swan::Ctx &ctx, Swan::Item::ActivateMeta meta) {
				Swan::Tile &tile = ctx.plane.tiles().get(meta.cursor);
				auto above = meta.cursor.add(0, -1);
				bool plantPotato =
					(tile.name == "core::dirt" || tile.name == "core::grass") &&
					(ctx.plane.tiles().getID(above) == Swan::World::AIR_TILE_ID);
				if (plantPotato) {
					ctx.plane.tiles().set(above, "core::potato-bush::0");
					meta.stack.remove(1);
				} else {
					foodItem<1>(ctx, meta);
				}
			},
		});
		registerItem({
			.name = "cooked-potato",
			.image = "core::items/cooked-potato",
			.onActivate = foodItem<3>,
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
			.inputs = {{1, "core::wood"}},
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
				{4, "core::wood"},
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
		registerRecipe({
			.inputs = {{3, "core::clay"}},
			.output = {1, "core::unfired-crucible"},
			.kind = "core::crafting",
		});

		registerRecipeKind("burning");
		registerRecipe({
			.inputs = {{1, "core::stick"}},
			.output = {},
			.kind = "core::burning",
		});
		registerRecipe({
			.inputs = {{1, "core::unfired-crucible"}},
			.output = {1, "core::crucible"},
			.kind = "core::burning",
		});
		registerRecipe({
			.inputs = {{1, "core::potato"}},
			.output = {1, "core::cooked-potato"},
			.kind = "core::burning",
		});
		registerRecipe({
			.inputs = {{1, "core::cooked-potato"}},
			.output = {1, "core::burned-food"},
			.kind = "core::burning",
		});
		registerRecipe({
			.inputs = {{1, "core::burned-food"}},
			.output = {},
			.kind = "core::burning",
		});

		registerRecipeKind("smelting");
		registerRecipe({
			.inputs = {{2, "core::iron-ore-chunk"}},
			.output = {2, "core::pig-iron"},
			.kind = "core::smelting",
		});
		registerRecipe({
			.inputs = {{2, "core::copper-ore-chunk"}},
			.output = {2, "core::copper"},
			.kind = "core::smelting",
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
