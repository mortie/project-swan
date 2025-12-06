#include "workbench.h"
#include "world/util.h"

using namespace std::literals;

namespace CoreMod {

static bool spawnWorkbench(Swan::Ctx &ctx, Swan::TilePos pos)
{
	if (pos.x + 0.5 < ctx.world.player_->center().x) {
		pos -= {1, 0};
	}

	if (!ctx.plane.tiles().get(pos).isReplacable()) {
		pos += {1, 0};
	}

	if (!ctx.plane.tiles().get(pos.add(1, 0)).isReplacable()) {
		pos -= {1, 0};
	}

	bool place =
		ctx.plane.tiles().get(pos).isReplacable() &&
		ctx.plane.tiles().get(pos.add(1, 0)).isReplacable() &&
		ctx.plane.tiles().get(pos.add(0, 1)).isSolid() &&
		ctx.plane.tiles().get(pos.add(1, 1)).isSolid();
	if (!place) {
		return false;
	}

	auto leftID = ctx.world.getTileID("core::workbench::left");
	auto rightID = ctx.world.getTileID("core::workbench::right");
	ctx.plane.tiles().setIDWithoutUpdate(pos, leftID);
	ctx.plane.tiles().setIDWithoutUpdate(pos.add(1, 0), rightID);
	ctx.plane.tiles().scheduleUpdate(pos);
	ctx.plane.tiles().scheduleUpdate(pos.add(1, 0));
	return true;
}

void updateLeft(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto &self = ctx.plane.tiles().get(pos);
	auto prefix = self.name.str();
	prefix.remove_suffix("::left"sv.size());

	auto &right = ctx.plane.tiles().get(pos.add(1, 0));
	if (right.name != Swan::cat(prefix, "::right")) {
		ctx.plane.tiles().breakTileSilently(pos);
		return;
	}

	breakIfFloating(ctx, pos);
}

void updateRight(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto &self = ctx.plane.tiles().get(pos);
	auto prefix = self.name.str();
	prefix.remove_suffix("::right"sv.size());

	auto &left = ctx.plane.tiles().get(pos.add(-1, 0));
	if (left.name != Swan::cat(prefix, "::left")) {
		ctx.plane.tiles().breakTileSilently(pos);
		return;
	}

	breakIfFloating(ctx, pos);
}

void registerWorkbench(Swan::Mod &mod)
{
	mod.registerTile({
		.name = "workbench",
		.image = "core::items/workbench",
		.isSolid = false,
		.isReplacable = true,
		.breakableBy = Swan::Tool::HAND,
		.onSpawn = spawnWorkbench,
	});

	auto traits = std::make_shared<WorkbenchTileTrait>();
	mod.registerTile({
		.name = "workbench::left",
		.image = "core::tiles/workbench@0",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::workbench",
		.onTileUpdate = updateLeft,
		.traits = traits,
	});
	mod.registerTile({
		.name = "workbench::right",
		.image = "core::tiles/workbench@1",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::workbench",
		.onTileUpdate = updateRight,
		.traits = traits,
	});
}

}
