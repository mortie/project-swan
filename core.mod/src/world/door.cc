#include "door.h"
#include "world/util.h"

using namespace std::literals;

namespace CoreMod {

static void setDoor(
	Swan::Ctx &ctx,
	Swan::TilePos pos,
	std::string_view prefix,
	std::string_view state)
{
	auto topID = ctx.world.getTileID(
		Swan::cat(prefix, "::", state, "::top"));
	auto bottomID = ctx.world.getTileID(
		Swan::cat(prefix, "::", state, "::bottom"));

	ctx.plane.tiles().setIDWithoutUpdate(pos, topID);
	ctx.plane.tiles().setIDWithoutUpdate(pos.add(0, 1), bottomID);
	ctx.plane.tiles().scheduleUpdate(pos);
	ctx.plane.tiles().scheduleUpdate(pos.add(0, 1));
}

static bool spawnDoor(Swan::Ctx &ctx, Swan::TilePos pos)
{
	const char *dir;
	if (pos.x + 0.5 < ctx.world.player_->center().x) {
		dir = "left";
	} else {
		dir = "right";
	}

	bool placeBottom =
		ctx.plane.tiles().get(pos.add(0, 1)).isSolid() &&
		ctx.plane.tiles().get(pos.add(0, -1)).isReplacable();
	if (placeBottom) {
		breakTileAndDropItem(ctx, pos.add(0, -1));
		setDoor(ctx, pos.add(0, -1), Swan::cat("core::door::", dir), "open");
		return true;
	}

	bool placeTop =
		ctx.plane.tiles().get(pos.add(0, 2)).isSolid() &&
		ctx.plane.tiles().get(pos.add(0, 1)).isReplacable() &&
		ctx.plane.tiles().get(pos.add(0, 0)).isReplacable();
	if (placeTop) {
		breakTileAndDropItem(ctx, pos.add(0, 1));
		setDoor(ctx, pos, Swan::cat("core::door::", dir), "open");
		return true;
	}

	return false;
}

static void updateTop(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto &self = ctx.plane.tiles().get(pos);
	auto prefix = self.name.str();
	prefix.remove_suffix("::top"sv.size());

	auto &below = ctx.plane.tiles().get(pos.add(0, 1));
	if (below.name != Swan::cat(prefix, "::bottom")) {
		ctx.plane.tiles().breakTileSilently(pos);
	}
}

static void updateBottom(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto &self = ctx.plane.tiles().get(pos);
	auto prefix = self.name.str();
	prefix.remove_suffix("::bottom"sv.size());

	auto &above = ctx.plane.tiles().get(pos.add(0, -1));
	if (above.name != Swan::cat(prefix, "::top")) {
		ctx.plane.tiles().breakTileSilently(pos);
		return;
	}

	breakIfFloating(ctx, pos);
}

static void activateOpenTop(
	Swan::Ctx &ctx,
	Swan::TilePos pos,
	Swan::Tile::ActivateMeta)
{
	auto &self = ctx.plane.tiles().get(pos);
	auto prefix = self.name.str();
	prefix.remove_suffix("::open::top"sv.size());
	setDoor(ctx, pos, prefix, "closed");
	ctx.game.playSound(ctx.world.getSound("core::misc/lock-close"), pos);
}

static void activateOpenBottom(
	Swan::Ctx &ctx,
	Swan::TilePos pos,
	Swan::Tile::ActivateMeta meta)
{
	activateOpenTop(ctx, pos.add(0, -1), meta);
}

static void activateClosedTop(
	Swan::Ctx &ctx,
	Swan::TilePos pos,
	Swan::Tile::ActivateMeta)
{
	auto &self = ctx.plane.tiles().get(pos);
	auto prefix = self.name.str();
	prefix.remove_suffix("::closed::top"sv.size());
	setDoor(ctx, pos, prefix, "open");
	ctx.game.playSound(ctx.world.getSound("core::misc/lock-open"), pos);
}

static void activateClosedBottom(
	Swan::Ctx &ctx,
	Swan::TilePos pos,
	Swan::Tile::ActivateMeta meta)
{
	activateClosedTop(ctx, pos.add(0, -1), meta);
}

void registerDoor(Swan::Mod &mod)
{
	mod.registerTile({
		.name = "door",
		.image = "core::items/door",
		.isSolid = false,
		.isReplacable = true,
		.breakableBy = Swan::Tool::HAND,
		.onSpawn = spawnDoor,
	});

	auto leftFluid = std::make_shared<Swan::FluidCollision>(
			0b1000'1000'1000'1000);
	auto rightFluid = std::make_shared<Swan::FluidCollision>(
			0b0001'0001'0001'0001);

	for (auto dir: {"left", "right"}) {
		auto fluid = [&] {
			if (dir == "left"sv) return leftFluid;
			else return rightFluid;
		}();

		mod.registerTile({
			.name = Swan::cat("door::", dir, "::open::top"),
			.image = Swan::cat("core::tiles/door/open::", dir, "@0"),
			.isSolid = false,
			.breakableBy = Swan::Tool::HAND,
			.droppedItem = "core::door",
			.onTileUpdate = updateTop,
			.onActivate = activateOpenTop,
		});
		mod.registerTile({
			.name = Swan::cat("door::", dir, "::open::bottom"),
			.image = Swan::cat("core::tiles/door/open::", dir, "@1"),
			.isSolid = false,
			.breakableBy = Swan::Tool::HAND,
			.droppedItem = "core::door",
			.onTileUpdate = updateBottom,
			.onActivate = activateOpenBottom,
		});

		mod.registerTile({
			.name = Swan::cat("door::", dir, "::closed::top"),
			.image = Swan::cat("core::tiles/door/closed::", dir, "@0"),
			.isOpaque = false,
			.breakableBy = Swan::Tool::HAND,
			.droppedItem = "core::door",
			.onTileUpdate = updateTop,
			.onActivate = activateClosedTop,
			.fluidCollision = fluid,
		});
		mod.registerTile({
			.name = Swan::cat("door::", dir, "::closed::bottom"),
			.image = Swan::cat("core::tiles/door/closed::", dir, "@1"),
			.isOpaque = false,
			.breakableBy = Swan::Tool::HAND,
			.droppedItem = "core::door",
			.onTileUpdate = updateBottom,
			.onActivate = activateClosedBottom,
			.fluidCollision = fluid,
		});
	}
}

}
