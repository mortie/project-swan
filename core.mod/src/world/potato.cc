#include "potato.h"
#include "util.h"

namespace CoreMod {

template<int N, int Max = N>
void registerPotatoBush(Swan::Mod &mod)
{
	if constexpr (N == Max) {
		mod.registerTile({
			.name = Swan::cat("potato-bush"),
			.image = Swan::cat("core::tiles/flora/potato-bush@", N),
			.isSolid = false,
			.breakableBy = Swan::Tool::HAND,
			.breakSound = "core::step/grass1",
			.droppedItem = "core::potato",
			.onBreak = dropRandomItemCount<"core::potato", 5>,
			.onTileUpdate = breakIfFloating,
		});
	} else {
		mod.registerTile({
			.name = Swan::cat("potato-bush::", N),
			.image = Swan::cat("core::tiles/flora/potato-bush@", N),
			.isSolid = false,
			.breakableBy = Swan::Tool::HAND,
			.breakSound = "core::step/grass1",
			.droppedItem = "core::potato",
			.onTileUpdate = breakIfFloating,
			.onWorldTick = +[](Swan::Ctx &ctx, Swan::TilePos pos) {
				if (Swan::random() % 4 != 0) {
					return;
				}

				if constexpr (N == Max - 1) {
					ctx.plane.tiles().set(pos, "core::potato-bush");
				} else {
					ctx.plane.tiles().set(pos, Swan::cat(
						"core::potato-bush::", N + 1));
				}
			},
		});
	}

	if constexpr (N > 0) {
		registerPotatoBush<N - 1, Max>(mod);
	}
}

void registerPotato(Swan::Mod &mod)
{
	registerPotatoBush<4>(mod);
}

}
