#include <swan/swan.h>

#include "DefaultWorldGen.h"
#include "entities/PlayerEntity.h"
#include "entities/ItemStackEntity.h"

class CoreMod: public Swan::Mod {
public:
	CoreMod(Swan::World &world): Swan::Mod("core") {
		break_listener_ = world.evt_tile_break_.subscribe(
			std::bind_front(&CoreMod::onTileBreak, this));

		registerImage("tile/air");
		registerImage("tile/stone");
		registerImage("tile/dirt");
		registerImage("tile/grass");
		registerImage("tile/tree-trunk");
		registerImage("tile/leaves");
		registerImage("entity/player-running");
		registerImage("entity/player-still");
		registerImage("misc/background-cave");

		registerTile({
			.name = "air",
			.image = "core/tile/air",
			.is_solid = false,
		});
		registerTile({
			.name = "stone",
			.image = "core/tile/stone",
			.dropped_item = "core::stone",
		});
		registerTile({
			.name = "dirt",
			.image = "core/tile/dirt",
			.dropped_item = "core::dirt",
		});
		registerTile({
			.name = "grass",
			.image = "core/tile/grass",
			.dropped_item = "core::dirt",
		});
		registerTile({
			.name = "tree-trunk",
			.image = "core/tile/tree-trunk",
			.dropped_item = "core::tree-trunk",
		});
		registerTile({
			.name = "leaves",
			.image = "core/tile/leaves",
		});

		registerItem({
			.name = "stone",
			.image = "core/tile/stone",
		});
		registerItem({
			.name = "dirt",
			.image = "core/tile/dirt",
		});
		registerItem({
			.name = "grass",
			.image = "core/tile/grass",
		});
		registerItem({
			.name = "tree-trunk",
			.image = "core/tile/tree-trunk",
		});

		registerWorldGen<DefaultWorldGen>("default");

		registerEntity<PlayerEntity>("player");
		registerEntity<ItemStackEntity>("item-stack");
	}

	void onTileBreak(const Swan::Context &ctx, Swan::TilePos pos, Swan::Tile &tile) {
		if (tile.dropped_item_) {
			ctx.plane.spawnEntity(std::make_unique<ItemStackEntity>(
				ctx, pos, *tile.dropped_item_));
		}
	}

	Swan::EventListener break_listener_;
};

extern "C" std::unique_ptr<Swan::Mod> mod_create(Swan::World &world) {
	return std::make_unique<CoreMod>(world);
}
