#include "uiutil.h"
#include "Game.h"
#include "Item.h"

#include <algorithm>

namespace Swan {

namespace UI {

static float linearLine(float from, float to, float frac)
{
	return std::clamp(to * frac + from * (1 - frac), 0.0f, 255.0f);
}

static Cygnet::Color linearColor(Cygnet::Color from, Cygnet::Color to, float frac)
{
	return {
		.r = linearLine(from.r, to.r, frac),
		.g = linearLine(from.g, to.g, frac),
		.b = linearLine(from.b, to.b, frac),
		.a = linearLine(from.a, to.a, frac),
	};
}

Cygnet::Color linearGradient(
	float val,
	std::initializer_list<std::pair<float, Cygnet::Color>> colors)
{
	const std::pair<float, Cygnet::Color> *arr = colors.begin();
	size_t size = colors.size();

	if (val < arr[0].first) {
		return arr[0].second;
	}

	for (size_t i = 1; i < size; ++i) {
		if (arr[i].first < val) {
			continue;
		}

		auto [fromv, fromc] = arr[i - 1];
		auto [tov, toc] = arr[i];
		float frac = (val - fromv) / (tov - fromv);
		return linearColor(fromc, toc, frac);
	}

	return arr[size - 1].second;
}

void inventory(
	Ctx &ctx, Cygnet::Renderer &rnd, Vec2i size, Cygnet::RenderSprite sprite,
	std::span<const ItemStack> content, int hovered)
{
	rnd.drawUIGrid({
		.sprite = sprite,
		.w = size.x,
		.h = size.y,
	});

	int x = 0;
	int y = 0;
	int index = 0;
	for (auto &stack: content) {
		if (hovered == index) {
			rnd.drawUIRect({
				.pos = {x + 1.0f, y + 1.0f},
				.size = {1, 1},
				.outline = {0, 0, 0, 0},
				.fill = {1, 1, 1, 0.5},
			}, Cygnet::Anchor::TOP_LEFT);
		}

		if (stack.empty()) {
			goto next;
		}

		rnd.drawUITile({
			.transform = Cygnet::Mat3gf{}
				.scale({0.6, 0.6})
				.translate({1.2, 1.2})
				.translate({float(x), float(y)}),
			.id = stack.item()->id,
		}, Cygnet::Anchor::TOP_LEFT);

		if (stack.count() >= 0) {
			// Draw item count for normal stacks
			rnd.drawUIText({
				.textCache = ctx.game.smallFont_,
				.pos = {x + 1.1f, y + 1.5f},
				.text = strify(stack.count()),
				.scale = 0.6,
			}, Cygnet::Anchor::TOP_LEFT);
		} else {
			// For stacks with a special negative count
			// (indicating that it's somehow unavailable),
			// overlay an "X"
			rnd.drawUIText({
				.textCache = ctx.game.smallFont_,
				.pos = {x + 1.1f, y + 1.5f},
				.text = "X",
				.scale = 0.8,
				.color = {0.7, 0.1, 0.2},
			}, Cygnet::Anchor::TOP_LEFT);
		}

	next:
		index += 1;
		x += 1;
		if (x >= size.x) {
			y += 1;
			x = 0;
			if (y >= size.y) {
				break;
			}
		}
	}

	for (; y < size.y; ++y) {
		for (; x < size.x; ++x) {
			rnd.drawUIText({
				.textCache = ctx.game.smallFont_,
				.pos = {x + 1.25f, y + 1.2f},
				.text = "X",
				.color = {1.0, 0.0, 0.0},
			}, Cygnet::Anchor::TOP_LEFT);
		}
		x = 0;
	}
}

Vec2i calcInventorySize(int size)
{
	if (size < 10) {
		return {size, 1};
	} else {
		int y = size / 10;
		if (size % 10) {
			y += 1;
		}
		return {10, y};
	}
}

Vec2 relativePos(Vec2 pos, Cygnet::Rect rect)
{
	pos -= rect.pos;
	pos += rect.size;
	return pos;
}

std::optional<Vec2i> inventoryCellPos(Vec2 pos, Cygnet::Rect rect)
{
	pos = relativePos(pos, rect);

	// The rectangle has a 2-tile padding
	pos -= {1, 1};
	rect.size *= 2;
	rect.size -= {2, 2};

	if (pos.x < 0 || pos.x > rect.size.x) {
		return std::nullopt;
	}
	if (pos.y < 0 || pos.y > rect.size.y) {
		return std::nullopt;
	}

	return pos.as<int>();
}

int inventoryCellIndex(Vec2 pos, Cygnet::Rect rect, int offset)
{
	auto cellPos = inventoryCellPos(pos, rect);
	if (!cellPos) {
		return -1;
	}

	auto size = (rect.size * 2).as<int>().add(-2, -2);
	return cellPos->y * (size.x) + cellPos->x + offset;
}

void tooltip(Ctx &ctx, Cygnet::Renderer &rnd, Vec2 pos, std::string_view text)
{
	float scale = 0.7;

	auto segment = rnd.prepareUIText({
		.textCache = ctx.game.smallFont_,
		.pos = pos,
		.text = text,
		.scale = scale,
	});
	segment.drawText.pos += {
		segment.size.x / 2 - 0.2f,
		-segment.size.y - 0.2f,
	};

	rnd.drawUIRect({
		.pos = pos.add(
			segment.size.x / 2 - 0.2f,
			-segment.size.y - 0.2f),
		.size = segment.size.add(0.2, 0.4),
		.outline = {0.3, 0.1, 0.1},
		.fill = {0, 0, 0, 0.7},
	});
	rnd.drawUIText(segment);
}

}

}
