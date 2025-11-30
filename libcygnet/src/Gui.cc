#include "Gui.h"
#include <iostream>
#include <vector>

namespace Cygnet {

struct Gui::Impl {
	Renderer *rnd;
	bool activated = false;
	Swan::Vec2 mousePos;
};

Gui::Gui(Renderer *rnd): impl_(std::make_unique<Impl>())
{
	impl_->rnd = rnd;
}

Gui::~Gui() = default;

Rect Gui::begin(Rect rect, Anchor anchor)
{
	return impl_->rnd->pushUIView(rect, anchor);
}

void Gui::end()
{
	impl_->rnd->popUIView();
}

bool Gui::button(TextCache &cache, std::string_view text)
{
	Rect frame = impl_->rnd->currentUiView();
	float padding = 0.5;

	auto segment = impl_->rnd->prepareUIText({
		.textCache = cache,
		.text = text,
	});

	auto half = segment.size.add(padding, padding) / 2;
	auto mouse = impl_->mousePos;
	bool collides =
		mouse.x >= frame.pos.x - half.x &&
		mouse.x <= frame.pos.x + half.x &&
		mouse.y >= frame.pos.y - half.y &&
		mouse.y <= frame.pos.y + half.y;

	Color color = {0.0, 0.0, 0.0, 0.9};
	if (collides) {
		color = {0.2, 0.3, 0.1, 0.9};
	}

	impl_->rnd->drawUIRect({
		.pos = {0, 0},
		.size = segment.size.add(padding, padding),
		.outline = {1.0, 1.0, 1.0, 1.0},
		.fill = color,
	});
	impl_->rnd->drawUIText(segment);

	return collides && impl_->activated;
}

void Gui::triggerActivate()
{
	impl_->activated = true;
}

void Gui::onMouseMove(Swan::Vec2 pos)
{
	impl_->mousePos = pos;
}

void Gui::moveSelectionLeft()
{
	// TODO
}

void Gui::moveSelectionRight()
{
	// TODO
}

void Gui::moveSelectionUp()
{
	// TODO
}

void Gui::moveSelectionDown()
{
	// TODO
}

void Gui::endFrame()
{
	impl_->activated = false;
}

}
