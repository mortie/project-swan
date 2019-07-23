#pragma once

#include <swan/swan.h>

class EntPlayer: public Swan::Entity {
public:
	class Factory: public Swan::Entity::Factory {
	public:
		Swan::Entity *create(const Swan::Vec2 &pos) override { return new EntPlayer(pos); }
	};

	Swan::Body body_;

	EntPlayer(Swan::Vec2 pos):
		body_(pos, SIZE, MASS) {}

	void draw(Swan::Win &win) override;
	void update(Swan::WorldPlane &plane, float dt) override;

private:
	static const float FORCE;
	static const float FRICTION;
	static const float MASS;
	static const Swan::Vec2 SIZE;
};
