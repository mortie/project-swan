#include <swan/swan.h>

#include "DefaultWorldGen.h"
#include "entities/PlayerEntity.h"
#include "entities/ItemStackEntity.h"
#include "entities/SpiderEntity.h"
#include "entities/FallingTileEntity.h"
#include "world/ladder.h"
#include "world/tree.h"
#include "world/util.h"

#include <thread>
#include <atomic>
#include <chrono>

namespace CoreMod {

static void musicThread(std::atomic<bool> *stop, Swan::World *world)
{
	using namespace std::chrono_literals;

	auto handle = Swan::SoundPlayer::makeHandle();

	while (!*stop) {
		auto asset = Swan::loadSoundAsset(world->modPaths_, "core::music/calm");
		if (!asset) {
			Swan::warn << "Failed to load core::music/calm: " << asset.err();
			return;
		}

		handle->done = false;
		world->game_->playSound(&asset.value(), handle);
		while (!*stop && !handle->done) {
			std::this_thread::sleep_for(100ms);
		}
	}
}

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
			.name = "sand",
			.image = "core::tiles/sand",
			.droppedItem = "core::sand",
			.onTileUpdate = fallIfFloating,
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
			.name = "rope-ladder",
			.image = "core::tiles/rope-ladder/anchor::left",
			.isSolid = false,
			.droppedItem = "core::rope-ladder-anchor",
			.onSpawn = spawnRopeLadderAnchor,
		});
		for (auto direction: {"left", "right"}) {
			registerTile({
				.name = Swan::cat("rope-ladder-anchor::", direction),
				.image = Swan::cat("core::tiles/rope-ladder/anchor::", direction),
				.isSolid = false,
				.droppedItem = "core::rope-ladder",
				.onTileUpdate = cascadeRopeLadder,
				.traits = std::make_shared<RopeLadderTileTrait>(true, direction),
			});
			registerTile({
				.name = Swan::cat("rope-ladder-middle::", direction),
				.image = Swan::cat("core::tiles/rope-ladder/middle::", direction),
				.isSolid = false,
				.onTileUpdate = cascadeRopeLadder,
				.traits = std::make_shared<RopeLadderTileTrait>(false, direction),
			});
			registerTile({
				.name = Swan::cat("rope-ladder-bottom::", direction),
				.image = Swan::cat("core::tiles/rope-ladder/bottom::", direction),
				.isSolid = false,
				.onTileUpdate = cascadeRopeLadder,
				.traits = std::make_shared<RopeLadderTileTrait>(false, direction),
			});
		}

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
	}

	~CoreMod()
	{
		stop_ = true;
		if (musicThread_.joinable()) {
			musicThread_.join();
		}
	}

	void start(Swan::World &world) override
	{
		musicThread_ = std::thread(musicThread, &stop_, &world);
	}

	std::atomic<bool> stop_;
	std::thread musicThread_;
};

}

extern "C" Swan::Mod *mod_create()
{
	return new CoreMod::CoreMod();
}
