#include "Chunk.h"

namespace Swan {

void Chunk::draw(Win &win) {
	if (dirty_) {
		sprite_.setTexture(texture_);
		dirty_ = false;
	}

	win.setPos(Vec2(pos_.x_ * CHUNK_WIDTH, pos_.y_ * CHUNK_HEIGHT));
	win.draw(sprite_);
}

}
