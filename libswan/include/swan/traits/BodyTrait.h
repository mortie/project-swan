#pragma once

#include "../common.h"

namespace Swan {

class Win;

struct BodyTrait {
	struct Body;
	struct Tag {};
	virtual Body &get(Tag) = 0;

	struct Body {
		Vec2 pos{};
		Vec2 size{};

		float left() { return pos.x; }
		float right() { return pos.x + size.x; }
		float midX() { return pos.x + size.x / 2; }
		float top() { return pos.y; }
		float bottom() { return pos.y + size.y; }
		float midY() { return pos.y + size.y / 2; }

		Vec2 topLeft() { return { left(), top() }; }
		Vec2 midLeft() { return { left(), midY() }; }
		Vec2 bottomLeft() { return { left(), bottom() }; }
		Vec2 topMid() { return { midX(), top() }; }
		Vec2 center() { return { midX(), midY() }; }
		Vec2 bottomMid() { return { midX(), bottom() }; }
		Vec2 topRight() { return { right(), top() }; }
		Vec2 midRight() { return { right(), midY() }; }
		Vec2 bottomRight() { return { right(), bottom() }; }

		void outline(Win &win);
	};
};

}
