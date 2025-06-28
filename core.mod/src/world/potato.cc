#include "potato.h"
#include "util.h"

namespace CoreMod {

template<int N, bool Last = true>
void registerPotatoBush(Swan::Mod &mod)
{
	if constexpr (Last) {
		mod.registerTile({
			.name = Swan::cat("potato-bush::", N),
			.image = Swan::cat("core::tiles/flora/potato-bush-", N),
			.isSolid = false,
			.breakableBy = Swan::Tool::HAND,
			.breakSound = "core::sounds/step/grass1",
			.droppedItem = "core::potato",
			.onBreak = dropRandomItemCount<"core::potato", 5>,
			.onTileUpdate = breakIfFloating,
		});
	} else {
		mod.registerTile({
			.name = Swan::cat("potato-bush::", N),
			.image = Swan::cat("core::tiles/flora/potato-bush-", N),
			.isSolid = false,
			.breakableBy = Swan::Tool::HAND,
			.breakSound = "core::sounds/step/grass1",
			.droppedItem = "core::potato",
			.onTileUpdate = breakIfFloating,
			.onWorldTick = +[](Swan::Ctx &ctx, Swan::TilePos pos) {
				if (Swan::random() % 4 != 0) {
					return;
				}

				ctx.plane.tiles().set(pos, Swan::cat(
					"core::potato-bush::", N + 1));
			},
		});
	}

	if constexpr (N > 0) {
		registerPotatoBush<N - 1, false>(mod);
	}
}

void registerPotato(Swan::Mod &mod)
{
	registerPotatoBush<4>(mod);
}

}
