#pragma once

#include <swan/swan.h>

namespace CoreMod {

void dropItem(
	Swan::Ctx &ctx, Swan::TilePos pos, Swan::Item &item);
void dropItem(
	Swan::Ctx &ctx, Swan::TilePos pos, const std::string &item);
void breakTileAndDropItem(Swan::Ctx &ctx, Swan::TilePos pos);

bool denyIfFloating(Swan::Ctx &ctx, Swan::TilePos pos);
void breakIfFloating(Swan::Ctx &ctx, Swan::TilePos pos);
void fallIfFloating(Swan::Ctx &ctx, Swan::TilePos pos);
void breakIfInFluid(Swan::Ctx &ctx, Swan::TilePos pos);

bool healPlayer(Swan::Ctx &ctx, Swan::EntityRef player, int n);

// Lookup table from 4-bit connection bitmap to offset into a 16-tile connected tile set.
// Bits: LEFT | RIGHT | UP | DOWN
extern const std::array<uint8_t, 16> CONNECTION_LUT_16;

// Lookup table from 8-bit connection bitmap to offset into a 47-tile connected tile set.
// Bits: LEFT | RIGHT | UP | DOWN
extern const std::array<uint8_t, 256> CONNECTION_LUT_47;

void registerConnected16(Swan::Mod &mod, Swan::Tile::Builder builder);
void registerConnected47(Swan::Mod &mod, Swan::Tile::Builder builder);
void registerBackgroundConnected47(Swan::Mod &mod, Swan::Tile::Builder builder);

void registerShovelable(Swan::Mod &mod, Swan::Tile::Builder builder);

template<int N>
void foodItem(Swan::Ctx &ctx, Swan::Item::ActivateMeta meta)
{
	if (healPlayer(ctx, meta.activator, N)) {
		meta.stack.remove(1);
	}
}

template<Swan::FixedString NAME, int MAX = 3, int NUM = 1, int DEN = 4>
void dropRandomItemCount(Swan::Ctx &ctx, Swan::TilePos pos)
{
	for (int i = 0; i < MAX; ++i) {
		if (Swan::randfloat() > (float(NUM) / float(DEN))) {
			dropItem(ctx, pos, NAME.cStr());
		}
	}
}

}
