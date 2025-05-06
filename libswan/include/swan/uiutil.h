#pragma once

#include "ItemStack.h"
#include "common.h"
#include <initializer_list>
#include <span>
#include <utility>
#include <cygnet/Renderer.h>
#include <cygnet/util.h>

namespace Swan {

namespace UI {

Cygnet::Color linearGradient(
	float val, std::initializer_list<std::pair<float, Cygnet::Color>> colors);

void inventory(
	const Context &ctx, Cygnet::Renderer &rnd, Vec2i size, Cygnet::RenderSprite sprite,
	std::span<ItemStack> content);

Vec2 relativePos(Vec2 pos, Cygnet::Renderer::Rect rect);

std::optional<Vec2i> inventoryCellPos(Vec2 pos, Cygnet::Renderer::Rect rect);

}

}
