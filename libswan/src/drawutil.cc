#include "drawutil.h"
#include "Game.h"
#include "Item.h"

#include <algorithm>

namespace Swan {

namespace Draw {

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
	const Context &ctx, Cygnet::Renderer &rnd, Vec2i size, Cygnet::RenderSprite sprite,
	std::span<ItemStack> content)
{
	rnd.drawUIGrid({
		.sprite = sprite,
		.w = size.x,
		.h = size.y,
	});

	int x = -1;
	int y = 0;
	for (auto &stack: content) {
		x += 1;
		if (x >= size.x) {
			y += 1;
			if (y >= size.y) {
				break;
			}
		}

		if (stack.empty()) {
			continue;
		}

		rnd.drawUITile({
			.transform = Cygnet::Mat3gf{}
				.scale({0.6, 0.6})
				.translate({1.2, 1.2})
				.translate({float(x), float(y)}),
			.id = stack.item()->id,
		});

		rnd.drawUIText({
			.textCache = ctx.game.smallFont_,
			.transform = Cygnet::Mat3gf{}
				.scale({0.7, 0.7})
				.translate({1.1, 1.5})
				.translate({float(x), float(y)}),
			.text = std::to_string(stack.count()).c_str(),
		});
	}

	for (; y < size.y; ++y) {
		for (; x < size.x; ++x) {
			rnd.drawUIText({
				.textCache = ctx.game.smallFont_,
				.transform = Cygnet::Mat3gf{}
					.translate({1.25, 1.2})
					.translate({float(x), float(y)}),
				.text = "X",
				.color = {1.0, 0.0, 0.0},
			});
		}
		x = 0;
	}
}

}

}
