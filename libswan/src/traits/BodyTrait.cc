#include "traits/BodyTrait.h"

#include "Win.h"

namespace Swan {

void BodyTrait::Body::outline(Win &win) {
	win.drawRect(pos, size);
}

}
