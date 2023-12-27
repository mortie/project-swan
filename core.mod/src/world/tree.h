#pragma once

#include "swan/swan.h"

struct TreeTrait: Swan::Tile::Traits {};

void spawnTree(const Swan::Context &ctx, Swan::TilePos pos);
void breakTree(const Swan::Context &ctx, Swan::TilePos pos);
