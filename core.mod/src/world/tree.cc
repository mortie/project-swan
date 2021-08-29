#include "tree.h"

#include <algorithm>

void spawnTree(const Swan::Context &ctx, Swan::TilePos pos) {
	Swan::Tile::ID logID = ctx.world.getTileID("core::tree-trunk");
	Swan::Tile::ID leavesID = ctx.world.getTileID("core::tree-leaves");
	int height = 4 + Swan::random(pos.x) % 3;
	for (int y = pos.y; y >= pos.y - height; --y) {
		ctx.plane.setTileID({pos.x, y}, logID);
	}

	int radius = 2 + Swan::random(pos.x) % 2;
	int radius2 = radius * radius;

	Swan::TilePos top = pos - Swan::Vec2i{0, height};
	for (int ry = -radius; ry <= radius + 2; ++ry) {
		for (int rx = -radius; rx <= radius; ++rx) {
			if (rx == 0 && ry <= -radius / 2) {
				continue;
			}

			int d2 = std::max(
					ry * ry + rx * rx,
					(ry + 1) * (ry + 1) + rx * rx);
			if (d2 <= radius2) {
				ctx.plane.setTileID(top - Swan::Vec2i{rx, ry}, leavesID);
			}
		}
	}
}
