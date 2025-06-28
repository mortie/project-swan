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

bool healPlayer(Swan::Ctx &ctx, Swan::EntityRef player, int n);

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
