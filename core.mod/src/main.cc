#include <swan/swan.h>

#include "DefaultWorldGen.h"
#include "entities/PlayerEntity.h"
#include "entities/ItemStackEntity.h"
#include "entities/SpiderEntity.h"
#include "world/ladder.h"
#include "world/tree.h"
#include "world/util.h"

namespace CoreMod {

class CoreMod: public Swan::Mod {
public:
	CoreMod(Swan::World &world): Swan::Mod("core")
	{
		registerSprite("entities/player/idle");
		registerSprite("entities/player/running");
		registerSprite("entities/player/falling");
		registerSprite("entities/player/jumping");
		registerSprite("entities/player/landing");
		registerSprite("entities/spider/idle");
		registerSprite("misc/background-cave");

		registerTile({
			.name = "stone",
			.image = "core::tiles/stone",
			.droppedItem = "core::stone",
		});
		registerTile({
			.name = "dirt",
			.image = "core::tiles/dirt",
			.droppedItem = "core::dirt",
		});
		registerTile({
			.name = "grass",
			.image = "core::tiles/grass",
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
			.name = "rope-ladder-anchor",
			.image = "core::tiles/rope-ladder/anchor-left",
			.droppedItem = "core::rope-ladder-anchor",
			.onSpawn = spawnRopeLadderAnchor,
		});
		registerTile({
			.name = "rope-ladder-anchor::left",
			.image = "core::tiles/rope-ladder/anchor-left",
			.isSolid = false,
			.droppedItem = "core::rope-ladder-anchor",
			.onTileUpdate = cascadeRopeLadder,
			.traits = std::make_shared<RopeLadderTileTrait>(true, "left"),
		});
		registerTile({
			.name = "rope-ladder-anchor::right",
			.image = "core::tiles/rope-ladder/anchor-right",
			.isSolid = false,
			.droppedItem = "core::rope-ladder-anchor",
			.onTileUpdate = cascadeRopeLadder,
			.traits = std::make_shared<RopeLadderTileTrait>(true, "right"),
		});
		registerTile({
			.name = "rope-ladder-middle::left",
			.image = "core::tiles/rope-ladder/middle-left",
			.isSolid = false,
			.onTileUpdate = cascadeRopeLadder,
			.traits = std::make_shared<RopeLadderTileTrait>(false, "left"),
		});
		registerTile({
			.name = "rope-ladder-middle::right",
			.image = "core::tiles/rope-ladder/middle-right",
			.isSolid = false,
			.onTileUpdate = cascadeRopeLadder,
			.traits = std::make_shared<RopeLadderTileTrait>(false, "right"),
		});
		registerTile({
			.name = "rope-ladder-bottom::left",
			.image = "core::tiles/rope-ladder/bottom-left",
			.isSolid = false,
			.onTileUpdate = cascadeRopeLadder,
			.traits = std::make_shared<RopeLadderTileTrait>(false, "left"),
		});
		registerTile({
			.name = "rope-ladder-bottom::right",
			.image = "core::tiles/rope-ladder/bottom-right",
			.isSolid = false,
			.onTileUpdate = cascadeRopeLadder,
			.traits = std::make_shared<RopeLadderTileTrait>(false, "right"),
		});

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
		registerRecipe({
			.inputs = {{2, "core::rope"}, {2, "core::wood-pole"}},
			.output = {1, "core::rope-ladder-anchor"},
			.kind = "crafting",
		});

		registerWorldGen<DefaultWorldGen>("default");

		registerEntity<PlayerEntity>("player");
		registerEntity<ItemStackEntity>("item-stack");
		registerEntity<SpiderEntity>("spider");
	}
};

}

extern "C" Swan::Mod *mod_create(Swan::World &world)
{
	return new CoreMod::CoreMod(world);
}
