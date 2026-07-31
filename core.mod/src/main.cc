#include <cstdlib>
#include <exception>
#include <memory>
#include <string>
#include <cpptoml.h>
#include <swan/swan.h>

#include "DefaultWorldGen.h"
#include "tiles.h"
#include "entities/DynamiteEntity.h"
#include "entities/PlayerEntity.h"
#include "entities/ItemStackEntity.h"
#include "entities/SpiderEntity.h"
#include "entities/FallingTileEntity.h"
#include "traits/items.h"
#include "world/aqueduct.h"
#include "world/bonfire.h"
#include "world/burner-generator.h"
#include "world/chest.h"
#include "world/clay.h"
#include "world/computer.h"
#include "world/copper-wire.h"
#include "world/door.h"
#include "world/drain.h"
#include "world/item-fan.h"
#include "world/ladder.h"
#include "world/lamps.h"
#include "world/outcrop.h"
#include "world/pipe.h"
#include "world/platform.h"
#include "world/potato.h"
#include "world/seeds.h"
#include "world/stairs.h"
#include "world/terrain.h"
#include "world/torch.h"
#include "world/tree.h"
#include "world/util.h"
#include "world/workbench.h"

namespace CoreMod {

class CoreMod: public Swan::Mod {
public:
	CoreMod(Swan::ModWrapper &w): Swan::Mod("core", w)
	{
		// Put this first,
		// so that everything that's part of the terrain
		// ends up with low tile IDs
		registerTerrain(*this);

		registerConnected16(*this, {
			.name = "glass",
			.image = "core::tiles/glass",
			.isOpaque = false,
			.breakableBy = Swan::Tool::HAND,
			.stepSound = "core::step/glass",
			.breakSound = "core::break/glass",
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
			.onBreak = [](Swan::Ctx &ctx, Swan::TilePos pos) {
				dropRandomItemCount<"core::fiber">(ctx, pos);
				spawnGrassSeed(ctx, pos);
			},
			.onTileUpdate = [](Swan::Ctx &ctx, Swan::TilePos pos) {
				auto below = pos.add(0, 1);
				if (ctx.plane.tiles().getID(below) != tiles::grass) {
					breakTileAndDropItem(ctx, pos);
				}
			},
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
			.name = "scorchbloom",
			.image = "core::tiles/flora/scorchbloom",
			.isSolid = false,
			.lightLevel = 20 / 255.0,
			.temperature = 0.5,
			.breakableBy = Swan::Tool::HAND,
			.placeSound = "core::place/leaves",
			.onBreak = [](Swan::Ctx &ctx, Swan::TilePos pos) {
				dropItem(ctx, pos, "core::scorchbloom-flower");
				spawnScorchbloomSeed(ctx, pos);
			},
			.onTileUpdate = breakIfFloating,
		});

		registerTile({
			.name = "scorchbloom-flower",
			.image = "core::tiles/flora/scorchbloom-flower",
			.isSolid = false,
			.lightLevel = 50 / 255.0,
			.onSpawn = [](Swan::Ctx &ctx, Swan::TilePos pos) { return false; },
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
			.name = "partial-water",
			.image = "@::invalid",
			.isSolid = false,
			.onSpawn = +[](Swan::Ctx &ctx, Swan::TilePos pos) {
				ctx.plane.tiles().setIDWithoutUpdate(pos, Swan::World::AIR_TILE_ID);
				ctx.plane.fluids().setPartialInTile(pos, ctx.world.getFluid("core::water").id);
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
		registerBurnerGenerator(*this);
		registerChest(*this);
		registerClay(*this);
		registerComputer(*this);
		registerCopperWire(*this);
		registerDoor(*this);
		registerDrain(*this);
		registerItemFan(*this);
		registerIncandescentLamp(*this);
		registerRopeLadder(*this);
		registerGlassPipe(*this);
		registerPlatform(*this);
		registerPotato(*this);
		registerSeedEntities(*this);
		registerStairs(*this);
		registerTorch(*this);
		registerScorchbloomTorch(*this);
		registerTree(*this);
		registerWorkbench(*this);

		registerItem({
			.name = "axe",
			.image = "core::items/tools/axe",
			.tool = Swan::Tool::AXE,
		});
		registerItem({
			.name = "shovel",
			.image = "core::items/tools/shovel",
			.tool = Swan::Tool::SHOVEL,
		});
		registerItem({
			.name = "dynamite",
			.image = "core::items/dynamite",
			.onActivate = [](Swan::Ctx &ctx, Swan::Item::ActivateMeta meta) {
				meta.stack.remove(1);
				auto pos = meta.activator.getBody()->topMid();
				auto vel = meta.direction * 15;
				ctx.plane.entities().spawn<DynamiteEntity>(pos, vel);
				return true;
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
			.name = "snowball",
			.image = "core::items/snowball",
		});
		registerItem({
			.name = "sand-pile",
			.image = "core::items/sand-pile",
		});
		registerItem({
			.name = "silt-pile",
			.image = "core::items/silt-pile",
		});
		registerItem({
			.name = "coal",
			.image = "core::items/coal",
			.traits = std::make_shared<BurnableItemTrait>(),
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
			.name = "crucible",
			.image = "core::items/crucible",
		});

		registerItem({
			.name = "burned-food",
			.image = "core::items/burned-food",
		});
		registerItem({
			.name = "potato",
			.image = "core::items/potato",
			.onActivate = +[](Swan::Ctx &ctx, Swan::Item::ActivateMeta meta) {
				Swan::Tile::ID tile = ctx.plane.tiles().getID(meta.cursor);
				auto above = meta.cursor.add(0, -1);
				bool plantPotato =
					(tile == tiles::dirt || tile == tiles::grass) &&
					(ctx.plane.tiles().getID(above) == Swan::World::AIR_TILE_ID);
				if (plantPotato) {
					ctx.plane.tiles().setID(above, tiles::potatoBush__0);
					meta.stack.remove(1);
					return true;
				}
				return false;
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
		registerRecipeKind("workbench");
		registerRecipeKind("burning");
		registerRecipeKind("smelting");

		// Load recipes from toml
		auto recipes = loadToml("recipes");
		for (auto &[kind, v]: *recipes) {
			auto array = v->as_table_array();
			if (!array) {
				Swan::warn << "Expected table array for " << kind;
				continue;
			}

			// Define this outside, so that we can use it for debug info
			// in the exception handler
			Swan::Recipe::Builder builder;
			builder.kind = kind;

			for (auto &recipe: *array) try {
				builder = {};
				builder.kind = kind;

				// Find the recipe output
				auto output = recipe->get("output")->as_array();
				auto outputItem = output->at(1)->as<std::string>();
				if (!outputItem) {
					throw std::runtime_error("Output is missing its item");
				}
				builder.output.item = std::move(outputItem->get());

				auto outputCount = output->at(0)->as<int64_t>();
				if (!outputCount) {
					throw std::runtime_error("Output is missing its count");
				}
				builder.output.count = int(outputCount->get());

				// Find the recipe inputs
				auto inputs = recipe->get("inputs")->as_array();
				for (auto &inputBase: *inputs) {
					auto input = inputBase->as_array();
					if (!input) {
						throw std::runtime_error("Input is not an array");
					}

					auto item = input->at(1)->as<std::string>();
					if (!item) {
						throw std::runtime_error("Input is missing its item");
					}

					auto count = input->at(0)->as<int64_t>();
					if (!count) {
						throw std::runtime_error("Input is missing its count");
					}
					builder.inputs.push_back({
						.count = int(count->get()),
						.item = std::move(item->get()),
					});
				}

				// Finally, register the recipe
				registerRecipe(std::move(builder));
			} catch (std::exception &ex) {
				if (!builder.output.item.empty()) {
					Swan::warn << "Bad " << kind << " recipe for " << builder.output.item;
				} else {
					Swan::warn << "Bad " << kind << " recipe";
				}
			}
		}
		recipes.reset();

		registerWorldGen<DefaultWorldGen>("default");

		registerEntity<DynamiteEntity>("dynamite");
		registerEntity<FallingTileEntity>("falling-tile");
		registerEntity<ItemStackEntity>("item-stack");
		registerEntity<PlayerEntity>("player");
		registerEntity<SpiderEntity>("spider");

		registerAction({
			.name = "cheat-heal",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:M"},
		});
		registerAction({
			.name = "cheat-hurt",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:N"},
		});
		registerAction({
			.name = "cheat-tick-world",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:P"},
		});
		registerAction({
			.name = "cheat-grab-item",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:G"},
		});

		registerAction({
			.name = "gui-show-inventory",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:E", "button:X"},
		});
		registerAction({
			.name = "gui-show-crafting",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:F", "button:Y"},
		});
		registerAction({
			.name = "gui-click",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"mouse:LEFT"},
		});
		registerAction({
			.name = "gui-modifier",
			.kind = Swan::ActionKind::CONTINUOUS,
			.defaultInputs = {"key:LEFT_SHIFT"},
		});
		registerAction({
			.name = "gui-left",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:LEFT", "button:DPAD_LEFT"},
		});
		registerAction({
			.name = "gui-right",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:RIGHT", "button:DPAD_RIGHT"},
		});
		registerAction({
			.name = "gui-up",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:UP", "button:DPAD_UP"},
		});
		registerAction({
			.name = "gui-down",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:DOWN", "button:DPAD_DOWN"},
		});

		registerAction({
			.name = "return-home",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:B", "button:RIGHT_THUMB"},
		});
		registerAction({
			.name = "break-tile",
			.kind = Swan::ActionKind::CONTINUOUS,
			.defaultInputs = {"mouse:LEFT", "button:RIGHT_BUMPER"},
		});
		registerAction({
			.name = "activate",
			.kind = Swan::ActionKind::CONTINUOUS,
			.defaultInputs = {"mouse:RIGHT", "button:LEFT_BUMPER"},
		});
		registerAction({
			.name = "drop-item",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:Q"},
		});
		registerAction({
			.name = "select-item",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:X", "button:B"},
		});
		registerAction({
			.name = "sprint",
			.kind = Swan::ActionKind::CONTINUOUS,
			.defaultInputs = {"key:LEFT_SHIFT", "button:LEFT_THUMB"},
		});
		registerAction({
			.name = "jump",
			.kind = Swan::ActionKind::CONTINUOUS,
			.defaultInputs = {"key:SPACE", "button:A"},
		});
		registerAction({
			.name = "move-x",
			.kind = Swan::ActionKind::AXIS,
			.defaultInputs = {"key:A;D", "axis:LEFT_X"},
		});
		registerAction({
			.name = "move-y",
			.kind = Swan::ActionKind::AXIS,
			.defaultInputs = {"key:W;S", "axis:LEFT_Y"},
		});
		registerAction({
			.name = "select-x",
			.kind = Swan::ActionKind::AXIS,
			.defaultInputs = {"axis:RIGHT_X"},
		});
		registerAction({
			.name = "select-y",
			.kind = Swan::ActionKind::AXIS,
			.defaultInputs = {"axis:RIGHT_Y"},
		});

		registerAction({
			.name = "select-slot-0",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:1"},
		});
		registerAction({
			.name = "select-slot-1",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:2"},
		});
		registerAction({
			.name = "select-slot-2",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:3"},
		});
		registerAction({
			.name = "select-slot-3",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:4"},
		});
		registerAction({
			.name = "select-slot-4",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:5"},
		});
		registerAction({
			.name = "select-slot-5",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:6"},
		});
		registerAction({
			.name = "select-slot-6",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:7"},
		});
		registerAction({
			.name = "select-slot-7",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:8"},
		});
		registerAction({
			.name = "select-slot-8",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:9"},
		});
		registerAction({
			.name = "select-slot-9",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:0"},
		});

		registerAction({
			.name = "open-console",
			.kind = Swan::ActionKind::ONESHOT,
			.defaultInputs = {"key:SLASH"},
		});

		registerCommand({
			.pattern = {"time"},
			.help = "Set or get the time of day.",
			.handler = +[](Swan::Ctx &ctx, std::span<Swan::CowStr> argv, std::string &out) {
				auto *gen = dynamic_cast<DefaultWorldGen *>(ctx.plane.worldGen_.get());
				if (!gen) {
					out = "Unknown current world generator.";
					return;
				}

				char buf[8];
				snprintf(buf, sizeof(buf), "%.1f%%", gen->timeOfDay() * 100);
				out = buf;
			},
		});
		registerCommand({
			.pattern = {"time", "@time"},
			.help = "Set the time of day to @time (a value between 0 and 100).",
			.handler = +[](Swan::Ctx &ctx, std::span<Swan::CowStr> argv, std::string &out) {
				auto *gen = dynamic_cast<DefaultWorldGen *>(ctx.plane.worldGen_.get());
				if (!gen) {
					out = "Unknown current world generator.";
					return;
				}

				auto res = Swan::parseInt(argv[0]);
				if (!res) {
					out = res.err();
					return;
				}

				auto val = res.value();
				if (val < 0 || val > 100) {
					out = "Bad time parameter";
					return;
				}

				gen->setTimeOfDay(float(val) / 100);
				out = "Set time of day to ";
				out += std::to_string(val);
				out += '%';
			},
		});
		registerCommand({
			.pattern = {"tp", "@x", "@y"},
			.help = "Set the player position to (@x, @y). Use '-' for current coordinate.",
			.handler = +[](Swan::Ctx &ctx, std::span<Swan::CowStr> argv, std::string &out) {
				if (argv[0].str() != "-") {
					ctx.world.player_->setMidX(strtof(std::string(argv[0].str()).c_str(), nullptr));
				}
				if (argv[1].str() != "-") {
					ctx.world.player_->setMidY(strtof(std::string(argv[1].str()).c_str(), nullptr));
				}
			},
		});
	}

	void start(Swan::World &world)
	{
		tiles::init(world);
	}
};

}

SWAN_MOD_ENTRY_POINT(CoreMod::CoreMod)
