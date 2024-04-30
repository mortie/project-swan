#include "Automata.h"
#include "util.h"

#include <string.h>
#include <cygnet/Renderer.h>

namespace Swan {

void Automata::tick()
{
	for (auto &[pos, chunk]: chunks_) {
		tickChunk(pos, chunk);
	}

	for (auto &[pos, chunk]: chunks_) {
		chunk.modified = chunk.nextModified;
		chunk.nextModified = false;
	}

	bool progress = true;
	while (!moveEvents_.empty() && progress) {
		progress = false;
		for (size_t i = 0; i < moveEvents_.size(); ++i) {
			MoveEvent &evt = moveEvents_[i];
			if (*evt.src != evt.srcCell || *evt.dest != evt.destCell) {
				continue;
			}

			*evt.dest = evt.after;
			if (evt.dest != evt.src) {
				*evt.src = evt.destCell;
			}
			progress = true;
			moveEvents_[i] = moveEvents_.back();
			moveEvents_.pop_back();
			break;
		}
	}

	moveEvents_.clear();
}

void Automata::draw(ChunkPos pos, Cygnet::Renderer &rnd)
{
	auto it = chunks_.find(pos);
	if (it == chunks_.end()) {
		return;
	}

	auto isWater = [](Cell c) {
		return c == Cell::WATER || c == Cell::WATER_L || c == Cell::WATER_R;
	};

	Chunk &chunk = it->second;
	for (int y = 0; y < AM_CHUNK_WIDTH; ++y) {
		float rndY = pos.y * CHUNK_WIDTH + (y / (float)RESOLUTION);

		for (int x = 0; x < AM_CHUNK_HEIGHT; ++x) {
			Cell cell = chunk[y][x];
			if (cell == Cell::SOLID) {
				continue;
			}

			int adjacentWater = 0;
			if (cell == Cell::AIR) {
				if (y > 0 && isWater(chunk[y - 1][x])) {
					adjacentWater += 1;
				}

				if (y < AM_CHUNK_HEIGHT - 1 && isWater(chunk[y + 1][x])) {
					adjacentWater += 2;
				}

				if (x > 0 && isWater(chunk[y][x - 1])) {
					adjacentWater += 2;
				}

				if (x < AM_CHUNK_WIDTH - 1 && isWater(chunk[y][x + 1])) {
					adjacentWater += 2;
				}

				if (adjacentWater < 3) {
					continue;
				}
			}

			float rndX = pos.x * CHUNK_WIDTH + (x / (float)RESOLUTION);

			Cygnet::Renderer::DrawRect rect = {
				.pos = {rndX, rndY},
				.size = {1.0 / RESOLUTION, 1.0 / RESOLUTION},
			};

			if (cell == Cell::AIR) {
				float alpha = 0;
				switch (adjacentWater) {
				case 1: alpha = 0.2; break;
				case 2: alpha = 0.3; break;
				default: alpha = 0.4; break;
				}
				rect.fill = {0.137f * alpha, 0.535f * alpha, 0.852f * alpha, alpha};
			}
			else if (cell == Cell::WATER) {
				rect.fill = {0.137 * 0.5, 0.535 * 0.5, 0.852 * 0.5, 0.5};
			}
			else if (cell == Cell::WATER_L) {
				rect.fill = {0.117 * 0.5, 0.425 * 0.5, 0.742 * 0.5, 0.5};
			}
			else if (cell == Cell::WATER_R) {
				rect.fill = {0.157 * 0.5, 0.645 * 0.5, 0.962 * 0.5, 0.5};
			}

			rect.outline = rect.fill;
			rnd.drawRect(rect);
		}
	}
}

void Automata::setTile(TilePos pos, Cell value)
{
	ChunkPos cpos = tilePosToChunkPos(pos);
	ChunkRelPos rpos = tilePosToChunkRelPos(pos) * RESOLUTION;

	Chunk &chunk = getChunk(cpos);
	chunk.modified = true;
	for (int y = rpos.y; y < rpos.y + RESOLUTION; ++y) {
		for (int x = rpos.x; x < rpos.x + RESOLUTION; ++x) {
			chunk[y][x] = value;
		}
	}
}

Automata::Chunk &Automata::getChunk(ChunkPos cpos)
{
	auto it = chunks_.find(cpos);
	if (it == chunks_.end()) {
		Chunk &chunk = chunks_[cpos];
		memset(chunk.data, 0, sizeof(chunk.data));
		return chunk;
	}

	return it->second;
}

void Automata::tickChunk(ChunkPos pos, Chunk &chunk)
{
	constexpr int TOP = 0;
	constexpr int BOTTOM = AM_CHUNK_HEIGHT - 1;
	constexpr int LEFT = 0;
	constexpr int RIGHT = AM_CHUNK_WIDTH - 1;

	// Main body
	if (chunk.modified) {
		for (int y = TOP + 1; y <= BOTTOM - 1; ++y) {
			for (int x = LEFT + 1; x <= RIGHT - 1; ++x) {
				Window win = {
					.self = chunk[y][x],
					.left = chunk[y][x - 1],
					.right = chunk[y][x + 1],
					.above = chunk[y - 1][x],
					.below = chunk[y + 1][x],
				};
				if (rule(win)) {
					chunk.nextModified = true;
				}
			}
		}
	}

	// Top edge
	if (auto it = chunks_.find(pos.add(0, -1)); it != chunks_.end()) {
		Chunk &aboveChunk = it->second;
		if (chunk.modified || aboveChunk.modified) {
			for (int x = LEFT + 1; x <= RIGHT - 1; ++x) {
				Window win = {
					.self = chunk[TOP][x],
					.left = chunk[TOP][x - 1],
					.right = chunk[TOP][x + 1],
					.above = aboveChunk[BOTTOM][x],
					.below = chunk[TOP + 1][x],
				};
				if (rule(win)) {
					chunk.nextModified = true;
					aboveChunk.nextModified = true;
				}
			}
		}
	}

	// Bottom edge
	if (auto it = chunks_.find(pos.add(0, 1)); it != chunks_.end()) {
		Chunk &belowChunk = it->second;
		if (chunk.modified || belowChunk.modified) {
			for (int x = LEFT + 1; x <= RIGHT - 1; ++x) {
				Window win = {
					.self = chunk[BOTTOM][x],
					.left = chunk[BOTTOM][x - 1],
					.right = chunk[BOTTOM][x + 1],
					.above = chunk[BOTTOM][x],
					.below = belowChunk[TOP][x],
				};
				if (rule(win)) {
					chunk.nextModified = true;
					belowChunk.nextModified = true;
				}
			}
		}
	}

	// Left edge
	if (auto it = chunks_.find(pos.add(-1, 0)); it != chunks_.end()) {
		Chunk &leftChunk = it->second;
		if (chunk.modified || leftChunk.modified) {
			for (int y = TOP + 1; y <= BOTTOM - 1; ++y) {
				Window win = {
					.self = chunk[y][LEFT],
					.left = leftChunk[y][RIGHT],
					.right = chunk[y][LEFT + 1],
					.above = chunk[y - 1][LEFT],
					.below = chunk[y + 1][LEFT],
				};
				if (rule(win)) {
					chunk.nextModified = true;
					leftChunk.nextModified = true;
				}
			}
		}
	}

	// Right edge
	if (auto it = chunks_.find(pos.add(1, 0)); it != chunks_.end()) {
		Chunk &rightChunk = it->second;
		if (chunk.modified || rightChunk.modified) {
			for (int y = TOP; y <= BOTTOM - 1; ++y) {
				Window win = {
					.self = chunk[y][RIGHT],
					.left = chunk[y][RIGHT - 1],
					.right = rightChunk[y][LEFT],
					.above = chunk[y - 1][RIGHT],
					.below = chunk[y + 1][RIGHT],
				};
				if (rule(win)) {
					chunk.nextModified = true;
					rightChunk.nextModified = true;
				}
			}
		}
	}

	if (auto itAbove = chunks_.find(pos.add(0, -1)); itAbove != chunks_.end()) {
		Chunk &aboveChunk = itAbove->second;

		// Top left
		if (auto it = chunks_.find(pos.add(-1, 0)); it != chunks_.end()) {
			Chunk &leftChunk = it->second;
			if (chunk.modified || aboveChunk.modified || leftChunk.modified) {
				Window win = {
					.self = chunk[TOP][LEFT],
					.left = leftChunk[TOP][RIGHT],
					.right = chunk[TOP][LEFT + 1],
					.above = aboveChunk[BOTTOM][LEFT],
					.below = chunk[BOTTOM + 1][LEFT],
				};
				if (rule(win)) {
					chunk.nextModified = true;
					aboveChunk.nextModified = true;
					leftChunk.nextModified = true;
				}
			}
		}

		// Top right
		if (auto it = chunks_.find(pos.add(1, 0)); it != chunks_.end()) {
			Chunk &rightChunk = it->second;
			if (chunk.modified || aboveChunk.modified || rightChunk.modified) {
				Window win = {
					.self = chunk[TOP][RIGHT],
					.left = chunk[TOP][RIGHT - 1],
					.right = rightChunk[TOP][LEFT],
					.above = aboveChunk[BOTTOM][RIGHT],
					.below = chunk[TOP + 1][RIGHT],
				};
				if (rule(win)) {
					chunk.nextModified = true;
					aboveChunk.nextModified = true;
					rightChunk.nextModified = true;
				}
			}
		}
	}

	if (auto itBelow = chunks_.find(pos.add(0, 1)); itBelow != chunks_.end()) {
		Chunk &belowChunk = itBelow->second;

		// Bottom left
		if (auto it = chunks_.find(pos.add(-1, 0)); it != chunks_.end()) {
			Chunk &leftChunk = it->second;
			if (chunk.modified || belowChunk.modified || leftChunk.modified) {
				Window win = {
					.self = chunk[BOTTOM][LEFT],
					.left = leftChunk[BOTTOM][RIGHT],
					.right = chunk[BOTTOM][LEFT + 1],
					.above = chunk[BOTTOM - 1][LEFT],
					.below = belowChunk[TOP][LEFT],
				};
				if (rule(win)) {
					chunk.nextModified = true;
					belowChunk.nextModified = true;
					leftChunk.nextModified = true;
				}
			}
		}

		// Bottom right
		if (auto it = chunks_.find(pos.add(1, 0)); it != chunks_.end()) {
			Chunk &rightChunk = it->second;
			if (chunk.modified || belowChunk.modified || belowChunk.modified) {
				Window win = {
					.self = chunk[BOTTOM][RIGHT],
					.left = chunk[BOTTOM][RIGHT - 1],
					.right = rightChunk[BOTTOM][LEFT],
					.above = chunk[BOTTOM - 1][RIGHT],
					.below = belowChunk[TOP][RIGHT],
				};
				if (rule(win)) {
					chunk.nextModified = true;
					belowChunk.nextModified = true;
					rightChunk.nextModified = true;
				}
			}
		}
	}
}

bool Automata::rule(Window win)
{
	auto isWater = [](Cell cell) {
		return cell == Cell::WATER || cell == Cell::WATER_L || cell == Cell::WATER_R;
	};

	if (win.self == Cell::WATER) {
		if (win.below == Cell::AIR) {
			moveEvents_.push_back({
				.src = &win.self,
				.dest = &win.below,
			});
		}
		else if (
				win.below == Cell::WATER_L &&
				random() % 4 == 0) {
			moveEvents_.push_back({
				.src = &win.self,
				.dest = &win.self,
				.after = Cell::WATER_L,
			});
		}
		else if (
				win.below == Cell::WATER_R &&
				random() % 4 == 0) {
			moveEvents_.push_back({
				.src = &win.self,
				.dest = &win.self,
				.after = Cell::WATER_R,
			});
		}
		else if (
				win.left == Cell::AIR &&
				win.right == Cell::AIR) {
			moveEvents_.push_back({
				.src = &win.self,
				.dest = &win.self,
				.after = random() % 2 ? Cell::WATER_L : Cell::WATER_R,
			});
		}
		else if (win.left == Cell::AIR) {
			moveEvents_.push_back({
				.src = &win.self,
				.dest = &win.left,
				.after = Cell::WATER_L,
			});
		}
		else if (win.right == Cell::AIR) {
			moveEvents_.push_back({
				.src = &win.self,
				.dest = &win.right,
				.after = Cell::WATER_R,
			});
		}
		else {
			return false;
		}
	}
	else if (
			(win.self == Cell::WATER_L || win.self == Cell::WATER_R) &&
			win.below == Cell::AIR) {
		moveEvents_.push_back({
			.src = &win.self,
			.dest = &win.below,
		});
	}
	else if (win.self == Cell::WATER_L) {
		if (
				win.left == Cell::AIR &&
				win.right == Cell::AIR &&
				random() % 128 == 0) {
			moveEvents_.push_back({
				.src = &win.self,
				.dest = &win.self,
				.after = Cell::AIR,
			});
		}
		else if (win.left == Cell::SOLID) {
			moveEvents_.push_back({
				.src = &win.self,
				.dest = &win.self,
				.after = Cell::WATER_R,
			});
		}
		else if (
				isWater(win.left) &&
				random() % 4 == 0) {
			moveEvents_.push_back({
				.src = &win.self,
				.dest = &win.self,
				.after = Cell::WATER_R,
			});
		}
		else if (
				win.left == Cell::AIR &&
				(win.below != Cell::WATER_L || random() % 8 > 0)) {
			moveEvents_.push_back({
				.src = &win.self,
				.dest = &win.left,
			});
		}
		else if (random() % 16 == 0) {
			moveEvents_.push_back({
				.src = &win.self,
				.dest = &win.self,
				.after = Cell::WATER,
			});
		}
	}
	else if (win.self == Cell::WATER_R) {
		if (
				win.left == Cell::AIR &&
				win.right == Cell::AIR &&
				random() % 128 == 0) {
			moveEvents_.push_back({
				.src = &win.self,
				.dest = &win.self,
				.after = Cell::AIR,
			});
		}
		else if (win.right == Cell::SOLID) {
			moveEvents_.push_back({
				.src = &win.self,
				.dest = &win.self,
				.after = Cell::WATER_L,
			});
		}
		else if (
				isWater(win.right) &&
				random() % 4 == 0) {
			moveEvents_.push_back({
				.src = &win.self,
				.dest = &win.self,
				.after = Cell::WATER_L,
			});
		}
		else if (
				win.right == Cell::AIR &&
				(win.below != Cell::WATER_R || random() % 8 > 0)) {
			moveEvents_.push_back({
				.src = &win.self,
				.dest = &win.right,
			});
		}
		else if (random() % 16 == 0) {
			moveEvents_.push_back({
				.src = &win.self,
				.dest = &win.self,
				.after = Cell::WATER,
			});
		}
	}
	else {
		return false;
	}

	return true;
}

}
