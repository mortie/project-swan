#pragma once

#include "Vector2.h"

namespace Swan {

struct BoundingBox {
	Vec2 pos;
	Vec2 size;

	double left() { return pos.x; }
	double right() { return pos.x + size.x; }
	double top() { return pos.y; }
	double bottom() { return pos.y + size.y; }
};

}
