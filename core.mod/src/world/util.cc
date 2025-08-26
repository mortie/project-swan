#include "util.h"

#include "entities/ItemStackEntity.h"
#include "entities/PlayerEntity.h"
#include "entities/FallingTileEntity.h"

namespace CoreMod {

void dropItem(
	Swan::Ctx &ctx, Swan::TilePos pos, Swan::Item &item)
{
	ctx.plane.entities().spawn<ItemStackEntity>(
		(Swan::Vec2)pos + Swan::Vec2{0.5, 0.5}, &item);
}

void dropItem(
	Swan::Ctx &ctx, Swan::TilePos pos, const std::string &item)
{
	dropItem(ctx, pos, ctx.world.getItem(item));
}

void breakTileAndDropItem(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto &droppedItem = ctx.plane.tiles().get(pos).more->droppedItem;

	if (droppedItem) {
		dropItem(ctx, pos, *droppedItem);
	}

	ctx.plane.breakTile(pos);
}

bool denyIfFloating(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto below = pos + Swan::TilePos{0, 1};

	return ctx.plane.tiles().get(below).isSupportV();
}

void breakIfFloating(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto below = pos + Swan::TilePos{0, 1};

	if (!ctx.plane.tiles().get(below).isSupportV()) {
		breakTileAndDropItem(ctx, pos);
	}
}

void fallIfFloating(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto below = pos + Swan::TilePos{0, 1};

	if (!ctx.plane.tiles().get(below).isSupportV()) {
		auto &tile = ctx.plane.tiles().get(pos);
		ctx.plane.tiles().setID(pos, Swan::World::AIR_TILE_ID);
		ctx.plane.entities().spawn<FallingTileEntity>(
			(Swan::Vec2)pos + Swan::Vec2{0.5, 0.5}, tile.id);
	}
}

bool healPlayer(Swan::Ctx &ctx, Swan::EntityRef playerRef, int n)
{
	auto player = dynamic_cast<PlayerEntity *>(playerRef.get());
	if (!player) {
		return false;
	}

	return player->heal(ctx, n);
}

static constexpr std::array<const char *, 16> CONNECTION_LUT = []() {
	// LEFT | RIGHT | UP | DOWN
	std::array<const char *, 16> lut;
	lut[0b0000] = "";
	lut[0b1000] = "14";
	lut[0b0100] = "12";
	lut[0b0010] = "11";
	lut[0b0001] = "3";
	lut[0b1100] = "13";
	lut[0b0011] = "7";
	lut[0b1010] = "10";
	lut[0b1001] = "2";
	lut[0b0110] = "8";
	lut[0b0101] = "0";
	lut[0b1110] = "9";
	lut[0b1101] = "1";
	lut[0b1011] = "6";
	lut[0b0111] = "4";
	lut[0b1111] = "5";
	return lut;
}();

static void updateConnected(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto &tile = ctx.plane.tiles().get(pos);

	auto name = tile.name.str();

	auto baseName = name;
	size_t atIndex = name.find("::@");
	if (atIndex != name.npos) {
		baseName.remove_suffix(name.size() - atIndex);
	}

	auto isConnected = [&tiles = ctx.plane.tiles(), baseName](Swan::TilePos pos) {
		auto &tile = tiles.get(pos);
		if (tile.name == baseName) {
			return true;
		}

		if (!tile.name.str().starts_with(baseName)) {
			return false;
		}

		return tile.name.str().substr(baseName.size()).starts_with("::@");
	};

	int left = isConnected(pos.add(-1, 0));
	int right = isConnected(pos.add(1, 0));
	int up = isConnected(pos.add(0, -1));
	int down = isConnected(pos.add(0, 1));
	int connection =
		(left << 3) |
		(right << 2) |
		(up << 1) |
		(down << 0);

	const char *desiredDir = CONNECTION_LUT[connection];
	std::string buf;
	auto desiredName = baseName;
	if (desiredDir[0]) {
		buf = Swan::cat(baseName, "::@", desiredDir);
		desiredName = buf;
	}

	Swan::info << pos << ": " << desiredName;
	if (desiredName != name) {
		ctx.plane.tiles().set(pos, desiredName);
	}
}

void registerConnected(Swan::Mod &mod, Swan::Tile::Builder builder)
{
	// We will be overwriting these, so they should be null
	assert(!builder.onTileUpdate);

	auto name = builder.name;
	auto image = builder.image;

	builder.onTileUpdate = updateConnected;

	// Unconnected
	builder.image = Swan::cat(image, "@15");
	mod.registerTile(Swan::Tile::Builder(builder));

	// The rest
	for (int i = 0; i < 15; ++i) {
		builder.name = Swan::cat(name, "::@", i);
		builder.image = Swan::cat(image, "@", i);
		mod.registerTile(Swan::Tile::Builder(builder));
	}
}

}
