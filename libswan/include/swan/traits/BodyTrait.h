#pragma once

#include "../common.h"

namespace Swan {

struct BodyTrait {
	struct Tag {};

	struct Body final {
		Vec2 pos{};
		Vec2 size{};
		bool isSolid = true;

		// The chunkPos is managed by the engine (notably, EntityCollection)
		ChunkPos chunkPos{};

		float left() const { return pos.x; }
		void setLeft(float x) { pos.x = x; }

		float right() const { return pos.x + size.x; }
		void setRight(float x) { pos.x = x - size.x; }

		float midX() const { return pos.x + size.x / 2; }
		void setMidX(float x) { pos.x = x - size.x / 2; }

		float top() const { return pos.y; }
		void setTop(float y) { pos.y = y; }

		float bottom() const { return pos.y + size.y; }
		void setBottom(float y) { pos.y = y - size.y; }

		float midY() const { return pos.y + size.y / 2; }
		void setMidY(float y) { pos.y = y - size.y / 2; }

		Vec2 topLeft() const { return {left(), top()}; }
		Vec2 midLeft() const { return {left(), midY()}; }
		Vec2 bottomLeft() const { return {left(), bottom()}; }

		Vec2 topMid() const { return {midX(), top()}; }
		Vec2 center() const { return {midX(), midY()}; }
		Vec2 bottomMid() const { return {midX(), bottom()}; }

		Vec2 topRight() const { return {right(), top()}; }
		Vec2 midRight() const { return {right(), midY()}; }
		Vec2 bottomRight() const { return {right(), bottom()}; }

		bool collidesWith(const Body &other) const
		{
			return
				(right() > other.left() && left() < other.right()) &&
				(bottom() > other.top() && top() < other.bottom());
		}
	};

	virtual Body &get(Tag) = 0;

protected:
	~BodyTrait() = default;
};

}
