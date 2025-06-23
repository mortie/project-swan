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
	Ctx &ctx, Cygnet::Renderer &rnd, Vec2i size, Cygnet::RenderSprite sprite,
	std::span<const ItemStack> content, int hovered = -1);

Vec2i calcInventorySize(int size);

Vec2 relativePos(Vec2 pos, Cygnet::Renderer::Rect rect);

std::optional<Vec2i> inventoryCellPos(Vec2 pos, Cygnet::Renderer::Rect rect);
int inventoryCellIndex(Vec2 pos, Cygnet::Renderer::Rect rect, int offset = 0);

void tooltip(Ctx &ctx, Cygnet::Renderer &rnd, std::string_view text);

}

}
