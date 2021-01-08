#include <swan/swan.h>

#include "DefaultWorldGen.h"
#include "entities/PlayerEntity.h"
#include "entities/ItemStackEntity.h"

class CoreMod: public Swan::Mod {
public:
	CoreMod(Swan::World &world): Swan::Mod("core") {
		breakListener_ = world.evtTileBreak_.subscribe(
			std::bind_front(&CoreMod::onTileBreak, this));

		registerImage("tile/stone");
		registerImage("tile/dirt");
		registerImage("tile/grass");
		registerImage("tile/tree-trunk");
		registerImage("tile/leaves");
		registerImage("tile/torch");
		registerImage("entity/player-running");
		registerImage("entity/player-still");
		registerImage("misc/background-cave");

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
		registerTile({
			.name = "torch",
			.image = "core/tile/torch",
			.is_solid = false,
			.light_level = 80/255.0,
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
		if (tile.droppedItem_) {
			ctx.plane.spawnEntity<ItemStackEntity>(
				ctx, (Swan::Vec2)pos + Swan::Vec2{0.5, 0.5}, *tile.droppedItem_);
		}
	}

	Swan::EventListener breakListener_;
};

extern "C" Swan::Mod *mod_create(Swan::World &world) {
	return new CoreMod(world);
}
