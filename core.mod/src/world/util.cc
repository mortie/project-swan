#include "util.h"

#include "entities/ItemStackEntity.h"
#include "entities/PlayerEntity.h"
#include "entities/FallingTileEntity.h"
#include "swan/common.h"
#include <utility>

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

void breakIfInFluid(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto fluid = ctx.plane.fluids().getAtPos(
		pos.as<float>().add(Swan::randfloat(0.4, 0.6), 0.5));
	if (fluid.density > 0) {
		breakTileAndDropItem(ctx, pos);
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

// Look-up table for basic 4x4 connected textures
constexpr std::array<uint8_t, 16> CONNECTION_LUT_16 = []() {
	// LEFT | RIGHT | UP | DOWN
	std::array<uint8_t, 16> lut;
	lut[0b0000] = 0;
	lut[0b1000] = 15;
	lut[0b0100] = 13;
	lut[0b0010] = 12;
	lut[0b0001] = 4;
	lut[0b1100] = 14;
	lut[0b0011] = 8;
	lut[0b1010] = 11;
	lut[0b1001] = 3;
	lut[0b0110] = 9;
	lut[0b0101] = 1;
	lut[0b1110] = 10;
	lut[0b1101] = 2;
	lut[0b1011] = 7;
	lut[0b0111] = 5;
	lut[0b1111] = 6;
	return lut;
}();

// Look-up table for basic 5x5 connected textures
constexpr std::array<uint8_t, 256> CONNECTION_LUT_47 = []() {
	std::array<uint8_t, 256> lut;
	for (int i = 0; i < 16; ++i) {
		for (int j = 0; j < 16; ++j) {
			lut[(i << 4) | j] = CONNECTION_LUT_16[i];
		}
	}

	auto set = [&](int lead, const char *var, uint8_t val) {
		int an = var[0] - '0';
		bool ax = var[0] == 'x';
		int bn = var[1] - '0';
		bool bx = var[1] == 'x';
		int cn = var[2] - '0';
		bool cx = var[2] == 'x';
		int dn = var[3] - '0';
		bool dx = var[3] == 'x';

		// Ugh I cba to fix this mess
		for (int a = 0; a <= 1; ++a) {
			if (!ax && a != an) continue;
			for (int b = 0; b <= 1; ++b) {
				if (!bx && b != bn) continue;
				for (int c = 0; c <= 1; ++c) {
					if (!cx && c != cn) continue;
					for (int d = 0; d <= 1; ++d) {
						if (!dx && d != dn) continue;
						lut[(lead << 4) | (a << 3) | (b << 2) | (c << 1) | d] = val;
					}
				}
			}
		}
	};

	// LEFT | RIGHT | UP | DOWN ... TL | TR | BL | BR
	set(0b0110, "00xx", 35);
	set(0b1010, "00xx", 38);
	set(0b1001, "xx0x", 19);
	set(0b0101, "xxx0", 16);

	set(0b1101, "xx00", 20);
	set(0b1110, "00xx", 39);
	set(0b1011, "0x0x", 45);
	set(0b0111, "x0x0", 42);

	set(0b1110, "10xx", 36);
	set(0b1110, "01xx", 37);
	set(0b1101, "xx10", 17);
	set(0b1101, "xx01", 18);

	set(0b1011, "0x1x", 31);
	set(0b1011, "1x0x", 25);
	set(0b0111, "x0x1", 28);
	set(0b0111, "x1x0", 22);

	lut[0b1111'1000] = 41;
	lut[0b1111'0100] = 40;
	lut[0b1111'0010] = 34;
	lut[0b1111'0001] = 33;

	lut[0b1111'0011] = 32;
	lut[0b1111'1100] = 26;
	lut[0b1111'1010] = 43;
	lut[0b1111'0101] = 44;

	lut[0b1111'1011] = 29;
	lut[0b1111'1110] = 23;
	lut[0b1111'0111] = 30;
	lut[0b1111'1101] = 24;

	lut[0b1111'0000] = 46;

	return lut;
}();

static void updateConnected16(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto &tile = ctx.plane.tiles().get(pos);

	Swan::Tile::ID baseID = tile.id - tile.more->baseOffset;

	auto isConnected = [&tiles = ctx.plane.tiles(), baseID](Swan::TilePos pos) {
		auto &tile = tiles.get(pos);
		return tile.id - tile.more->baseOffset == baseID;
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

	Swan::Tile::ID desiredID = baseID + CONNECTION_LUT_16[connection];
	if (tile.id == desiredID) {
		return;
	}

	ctx.plane.tiles().setID(pos, desiredID);
}

static void updateConnected47(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto &tile = ctx.plane.tiles().get(pos);

	Swan::Tile::ID baseID = tile.id - tile.more->baseOffset;

	auto isConnected = [&tiles = ctx.plane.tiles(), baseID](Swan::TilePos pos) {
		auto &tile = tiles.get(pos);
		return tile.id - tile.more->baseOffset == baseID;
	};

	int left = isConnected(pos.add(-1, 0));
	int right = isConnected(pos.add(1, 0));
	int up = isConnected(pos.add(0, -1));
	int down = isConnected(pos.add(0, 1));
	int tl = isConnected(pos.add(-1, -1));
	int tr = isConnected(pos.add(1, -1));
	int bl = isConnected(pos.add(-1, 1));
	int br = isConnected(pos.add(1, 1));
	int connection =
		(left << 7) |
		(right << 6) |
		(up << 5) |
		(down << 4) |
		(tl << 3) |
		(tr << 2) |
		(bl << 1) |
		(br << 0);

	Swan::Tile::ID desiredID = baseID + CONNECTION_LUT_47[connection];
	if (tile.id == desiredID) {
		return;
	}

	ctx.plane.tiles().setID(pos, desiredID);
}

static void updateBackgroundConnected47(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto &tile = ctx.plane.tiles().getBackground(pos);

	Swan::Tile::ID baseID = tile.id - tile.more->baseOffset;

	auto isConnected = [&tiles = ctx.plane.tiles()](Swan::TilePos pos) {
		Swan::Tile *t = tiles.maybeGet(pos);
		if (!t) {
			return false;
		}

		return !t->isSolid();
	};

	int left = isConnected(pos.add(-1, 0));
	int right = isConnected(pos.add(1, 0));
	int up = isConnected(pos.add(0, -1));
	int down = isConnected(pos.add(0, 1));
	int tl = isConnected(pos.add(-1, -1));
	int tr = isConnected(pos.add(1, -1));
	int bl = isConnected(pos.add(-1, 1));
	int br = isConnected(pos.add(1, 1));
	int connection =
		(left << 7) |
		(right << 6) |
		(up << 5) |
		(down << 4) |
		(tl << 3) |
		(tr << 2) |
		(bl << 1) |
		(br << 0);

	Swan::Tile::ID desiredID = baseID + CONNECTION_LUT_47[connection];
	if (tile.id == desiredID) {
		return;
	}

	ctx.plane.tiles().setBackgroundID(pos, desiredID);
}

static void registerConnectedTiles42(Swan::Mod &mod, Swan::Tile::Builder builder)
{
	auto name = builder.name;
	auto image = builder.image;

	// Unconnected
	builder.image = Swan::cat(image, "@36");
	mod.registerTile(Swan::Tile::Builder(builder));
	builder.baseOffset += 1;

	int nameIdx = 1;
	auto registerRange = [&](std::pair<int, int> xs, std::pair<int, int> ys) {
		for (int y = ys.first; y < ys.first + ys.second; ++y) {
			for (int x = xs.first; x < xs.first + xs.second; ++x) {
				builder.name = Swan::cat(name, "::@", nameIdx++);
				builder.image = Swan::cat(image, "@", y * 11 + x);
				mod.registerTile(Swan::Tile::Builder(builder));
				builder.baseOffset += 1;
			}
		}
	};

	// The rest of the 4x4
	registerRange({0, 4}, {0, 3});
	registerRange({0, 3}, {3, 1});

	// The rest of the 47
	registerRange({4, 6}, {0, 2});
	registerRange({4, 7}, {2, 2});
	registerRange({4, 5}, {4, 1});
}

void registerConnected16(Swan::Mod &mod, Swan::Tile::Builder builder)
{
	// We will be overwriting these, so they should be null
	assert(!builder.onTileUpdate);

	auto name = builder.name;
	auto image = builder.image;

	builder.onTileUpdate = updateConnected16;

	// Unconnected
	builder.image = Swan::cat(image, "@15");
	mod.registerTile(Swan::Tile::Builder(builder));
	builder.baseOffset += 1;

	// The rest
	for (int i = 0; i < 15; ++i) {
		builder.name = Swan::cat(name, "::@", i + 1);
		builder.image = Swan::cat(image, "@", i);
		mod.registerTile(Swan::Tile::Builder(builder));
		builder.baseOffset += 1;
	}
}

void registerConnected42(Swan::Mod &mod, Swan::Tile::Builder builder)
{
	// We will be overwriting these, so they should be null
	assert(!builder.onTileUpdate);

	builder.onTileUpdate = updateConnected47;
	registerConnectedTiles42(mod, builder);
}

void registerBackgroundConnected47(Swan::Mod &mod, Swan::Tile::Builder builder)
{
	// We will be overwriting these, so they should be null
	assert(!builder.onSpawn);
	assert(!builder.onTileUpdate);

	builder.isBackground = true;
	builder.onTileUpdate = updateBackgroundConnected47;
	builder.onSpawn = +[](Swan::Ctx &ctx, Swan::TilePos pos) {
		updateBackgroundConnected47(ctx, pos);
		return true;
	};
	registerConnectedTiles42(mod, builder);
}

template<bool Last = false>
void activateShovelable(Swan::Ctx &ctx, Swan::TilePos pos, Swan::Tile::ActivateMeta)
{
	auto &tile = ctx.plane.tiles().get(pos);
	auto *droppedItem = tile.more->droppedItem;
	if (droppedItem) {
		auto stack = ctx.plane.entities().spawn<ItemStackEntity>(
			pos.as<float>().add(0.5, -0.3), droppedItem);
		stack.trait<Swan::PhysicsBodyTrait>()->addVelocity({0, -3.0f});
	}

	ctx.game.playSound(tile.more->breakSound);

	if constexpr (Last) {
		ctx.plane.tiles().setID(pos, Swan::World::AIR_TILE_ID);
	} else {
		ctx.plane.tiles().setID(pos, tile.id + 1);
	}
}

void registerShovelable(Swan::Mod &mod, Swan::Tile::Builder builder)
{
	mod.registerTile(builder.clone()
		.withOnActivate(activateShovelable)
		.withImage(Swan::cat(builder.image, "@0")));
	mod.registerTile(builder.clone()
		.withOnActivate(activateShovelable)
		.withFluidCollision(std::make_shared<Swan::FluidCollision>(0b1111'1111'1111'0000))
		.withName(Swan::cat(builder.name, "::1"))
		.withImage(Swan::cat(builder.image, "@1"))
		.withIsSupportV(false)
		.withIsSupportH(false)
		.withIsSolid(false));
	mod.registerTile(builder.clone()
		.withOnActivate(activateShovelable)
		.withFluidCollision(std::make_shared<Swan::FluidCollision>(0b1111'1111'0000'0000))
		.withName(Swan::cat(builder.name, "::2"))
		.withImage(Swan::cat(builder.image, "@2"))
		.withIsSupportV(false)
		.withIsSupportH(false)
		.withIsSolid(false));
	mod.registerTile(builder.clone()
		.withOnActivate(activateShovelable<true>)
		.withFluidCollision(std::make_shared<Swan::FluidCollision>(0b1111'0000'0000'0000))
		.withName(Swan::cat(builder.name, "::3"))
		.withImage(Swan::cat(builder.image, "@3"))
		.withIsSupportV(false)
		.withIsSupportH(false)
		.withIsSolid(false));
}

}
